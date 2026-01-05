#include "luaModule/luaAT.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

LuaAT::LuaAT(QObject *parent)
    : QObject(parent) {
}

void LuaAT::exec(const std::string &portName, const std::string &command, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData = "AT+" + QByteArray::fromStdString(command);
    bool status = false;
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    }
}

void LuaAT::read(const std::string &portName, const std::string &command, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData = "AT+" + QByteArray::fromStdString(command) + "?";
    bool status = false;
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    }
}

void LuaAT::set(const std::string &portName, const std::string &command, const std::string &value, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData = "AT+" + QByteArray::fromStdString(command) + "=" + QByteArray::fromStdString(value);
    bool status = false;
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    }
}

void LuaAT::test(const std::string &portName, const std::string &command, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData = "AT+" + QByteArray::fromStdString(command) + "=?";
    bool status = false;
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "ascii", "crlf");
        }, Qt::BlockingQueuedConnection);
    }
}
