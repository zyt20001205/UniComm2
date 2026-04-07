#include "luaModule/port.h"

#include <sol/state_view.hpp>
#include <sol/table_core.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/uniCast.h"

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

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QVariantHash infoHash{};

    QMetaObject::invokeMethod(port, [&port, &infoHash] {
        infoHash = port->info();
    }, Qt::BlockingQueuedConnection);
    return uni_cast<sol::object>(ts, infoHash);
}

void Port::open(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    bool status = false;

    QMetaObject::invokeMethod(port, [&port, &status] {
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

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray txData(data.data(), static_cast<qsizetype>(data.size()));
    bool status = false;

    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &txData, &peerIp, &status] {
            status = port->write(txData, QString::fromStdString(peerIp), "", "");
        }, Qt::BlockingQueuedConnection);
    } else {
        QMetaObject::invokeMethod(port, [&port, &txData, &status] {
            status = port->write(txData, "", "");
        }, Qt::BlockingQueuedConnection);
    }
    if (!status) throw sol::error("failed to write port: " + portName);
}

sol::object Port::read(const sol::this_state ts, const std::string &portName, const int length, const int timeout, const std::string &peerIp) {
    sol::state_view lua(ts);
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray rxData{};

    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &length, &timeout, &peerIp, &rxData] {
            rxData = port->read(length, timeout, "", QString::fromStdString(peerIp));
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    if (port->type() == VIDEOSTREAM) {
        QMetaObject::invokeMethod(port, [&port, &length, &timeout, &rxData] {
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
    QMetaObject::invokeMethod(port, [&port, &timeout, &length, &rxData] {
        rxData = port->read(length, timeout, "");
    }, Qt::BlockingQueuedConnection);
    return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
}

sol::object Port::readUntil(const sol::this_state ts, const std::string &portName, const std::string &text, const int timeout, const std::string &peerIp) {
    sol::state_view lua(ts);
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray textData(text.data(), static_cast<qsizetype>(text.size()));
    QByteArray rxData{};

    if (port->type() == TCPSERVER) {
        QMetaObject::invokeMethod(port, [&port, &textData, &timeout, &peerIp, &rxData] {
            rxData = port->readUntil(textData, timeout, "", QString::fromStdString(peerIp));
        }, Qt::BlockingQueuedConnection);
        return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
    }
    if (port->type() == VIDEOSTREAM) {
        QMetaObject::invokeMethod(port, [&port, &textData, &timeout, &rxData] {
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
    QMetaObject::invokeMethod(port, [&port, &textData, &timeout, &rxData] {
        rxData = port->readUntil(textData, timeout, "");
    }, Qt::BlockingQueuedConnection);
    return sol::make_object(lua, std::string(rxData.constData(), static_cast<std::string::size_type>(rxData.size())));
}
