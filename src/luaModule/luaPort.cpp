#include "luaModule/luaPort.h"

#include <sol/sol.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

LuaPort::LuaPort(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaPort::list() {
    QSet<QString> portSet{};
    emit listPort(portSet);
    std::vector<std::string> portList{};
    for (const auto &portName: portSet) {
        portList.push_back(portName.toStdString());
    }
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

sol::object LuaPort::read(const sol::this_state ts, const std::string &portName, const int timeout, const int length, const std::string &peerIp) {
    sol::state_view lua(ts);
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &timeout, &length, &peerIp, &rxData] {
            rxData = port->read(timeout, length, "", QString::fromStdString(peerIp));
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    if (port->type() == VIDEOSTREAM) {
        QMetaObject::invokeMethod(port, [&port, &timeout, &length, &rxData] {
            rxData = port->read(timeout, length, "");
        }, Qt::BlockingQueuedConnection);
        if (rxData.contains('\x1E')) {
            sol::table table = lua.create_table();
            QList<QByteArray> parts = rxData.split('\x1E');
            for (int i = 0; i < parts.size(); ++i) {
                table[i + 1] = std::string(parts[i].constData(), parts[i].size());
            }
            return table;
        } else {
            return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
        }
    } else {
        QMetaObject::invokeMethod(port, [&port, &timeout, &length, &rxData] {
            rxData = port->read(timeout, length, "");
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
}
