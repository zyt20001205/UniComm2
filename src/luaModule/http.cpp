#include "luaModule/http.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

Http::Http(QObject *parent)
    : QObject(parent) {
}

void Http::get(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "GET /get HTTP/1.1\r\n";
    const auto host = port->info().value("remoteHost", "").toByteArray();
    if (host.isEmpty()) {
        throw sol::error(portName + ": Host not found");
    }
    txData += "Host: " + host + "\r\n";
    bool status = false;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        // port->clear();
        // rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    // if (!status || rxData.isEmpty()) {
    //     throw sol::error(portName + ": communication failed");
    // }
    // if (!QString::fromLatin1(rxData).startsWith("250")) {
    //     throw sol::error(portName + ": SMTP EHLO failed");
    // }
}
