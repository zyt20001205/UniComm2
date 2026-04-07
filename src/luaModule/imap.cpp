#include "luaModule/imap.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

// public
Imap::Imap(QObject *parent)
    : QObject(parent) {
}

void Imap::login(const std::string &portName, const std::string &username, const std::string &password) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &rxData] {
        rxData = port->read(3, 1000, "utf-8");
    }, Qt::BlockingQueuedConnection);
    if (rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    parse(rxData);
}

// private
void Imap::parse(const QByteArray &status) {
}
