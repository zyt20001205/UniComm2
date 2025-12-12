#include "luaModule/luaPort.h"

#include <sol/sol.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

LuaPort::LuaPort(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaPort::list() {
    std::vector<std::string> portList{};
    emit listPort(portList);
    return portList;
}

std::unordered_map<std::string, std::string> LuaPort::info(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    std::unordered_map<std::string, std::string> portInfo{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QMetaObject::invokeMethod(port, [&port, &portInfo] {
        portInfo = port->info();
    }, Qt::BlockingQueuedConnection);
    return portInfo;
}

void LuaPort::open(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    bool status = false;
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QMetaObject::invokeMethod(port, [&port, &status] {
        status = port->open();
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("failed to open port: " + portName);
    }
}

void LuaPort::close(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QMetaObject::invokeMethod(port, [&port] {
        port->close();
    }, Qt::BlockingQueuedConnection);
}

void LuaPort::write(const std::string &portName, const std::string_view &data, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    const QByteArray txData(data.data(), static_cast<qsizetype>(data.size()));
    bool status = false;
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "", "");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "", "");
        }, Qt::BlockingQueuedConnection);
    }
    if (!status) {
        throw sol::error("failed to write port: " + portName);
    }
}

std::string LuaPort::read(const std::string &portName, const int timeout, const int length, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &timeout, &length, &peerIp, &rxData] {
            rxData = port->read(timeout, length, "", QString::fromStdString(peerIp));
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &timeout, &length, &rxData] {
            rxData = port->read(timeout, length, "");
        }, Qt::BlockingQueuedConnection);
    }
    return {rxData.constData(), static_cast<std::string::size_type>(rxData.size())};
}
