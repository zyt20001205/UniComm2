#include "api/port.h"

#include <sol/state_view.hpp>
#include <sol/table_core.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

Port::Port(QObject *parent)
    : QObject(parent) {
}

sol::table Port::list(const sol::this_state ts) {
    QSet<QString> portSet{};

    QMetaObject::invokeMethod(g_port, [&portSet] {
        portSet = g_port->portList();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<sol::table>(ts, portSet);
}

sol::object Port::info(const sol::this_state ts, const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QVariantHash infoHash{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&infoHash, &port] {
        infoHash = port->info();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<sol::object>(ts, infoHash);
}

void Port::open(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    bool status = false;
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&status, &port] {
        status = port->open();
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error("failed to open port: " + portName);
}

void Port::close(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&port] {
        port->close();
    }, Qt::BlockingQueuedConnection);
}

void Port::clear(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&port] {
        port->clear();
    }, Qt::BlockingQueuedConnection);
}

void Port::write(const std::string &portName, const std::string &data, const std::string &peerIp) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    bool status = false;
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray txData(data.data(), static_cast<qsizetype>(data.size()));

    if (port->type() == PortType::TcpServer) {
        QMetaObject::invokeMethod(port, [&status, &port, &txData, &peerIp] {
            status = port->write(txData, QString::fromStdString(peerIp), "", "");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&status, &port, &txData] {
            status = port->write(txData, "", "");
        }, Qt::BlockingQueuedConnection);
    }
    if (!status) throw sol::error("failed to write port: " + portName);
}

sol::object Port::read(const sol::this_state ts, const std::string &portName, const int length, const int timeout, const std::string &peerIp) {
    sol::state_view lua(ts);
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    if (port->type() == PortType::TcpServer) {
        QMetaObject::invokeMethod(port, [&rxData, &port, &length, &timeout, &peerIp] {
            if (peerIp.empty()) rxData = port->read(length, timeout, "");
            else rxData = port->read(length, timeout, QString::fromStdString(peerIp), "");
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    if (port->type() == PortType::VideoStream) {
        QMetaObject::invokeMethod(port, [&rxData, &port, &length, &timeout] {
            rxData = port->read(length, timeout, "");
        }, Qt::BlockingQueuedConnection);
        if (rxData.contains('\x1E')) {
            sol::table table = lua.create_table();
            QList<QByteArray> parts = rxData.split('\x1E');
            for (int i = 0; i < parts.size(); ++i) {
                table[i + 1] = std::string(parts[i].constData(), parts[i].size());
            }
            return table;
        }
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    QMetaObject::invokeMethod(port, [&rxData, &port, &length, &timeout] {
        rxData = port->read(length, timeout, "");
    }, Qt::BlockingQueuedConnection);
    return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
}

sol::object Port::readUntil(const sol::this_state ts, const std::string &portName, const std::string &text, const int timeout, const std::string &peerIp) {
    sol::state_view lua(ts);
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray textData(text.data(), static_cast<qsizetype>(text.size()));

    if (port->type() == PortType::TcpServer) {
        QMetaObject::invokeMethod(port, [&rxData, &port, &textData, &timeout, &peerIp] {
            if (peerIp.empty()) rxData = port->readUntil(textData, timeout, "");
            else rxData = port->readUntil(textData, timeout, QString::fromStdString(peerIp), "");
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    if (port->type() == PortType::VideoStream) {
        QMetaObject::invokeMethod(port, [&rxData, &port, &textData, &timeout] {
            rxData = port->readUntil(textData, timeout, "");
        }, Qt::BlockingQueuedConnection);
        if (rxData.contains('\x1E')) {
            sol::table table = lua.create_table();
            QList<QByteArray> parts = rxData.split('\x1E');
            for (int i = 0; i < parts.size(); ++i) {
                table[i + 1] = std::string(parts[i].constData(), parts[i].size());
            }
            return table;
        }
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    QMetaObject::invokeMethod(port, [&rxData, &port, &textData, &timeout] {
        rxData = port->readUntil(textData, timeout, "");
    }, Qt::BlockingQueuedConnection);
    return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
}
