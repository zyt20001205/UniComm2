#include "api/http.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

Http::Http(QObject *parent)
    : QObject(parent) {
}

void Http::init(const std::string &portName, const int timeout) {
    const QString name = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(name);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");

    m_portName = portName;
    m_timeout = timeout;
    m_port = port.value();
}

void Http::get(const std::string &portName, const sol::table &headers, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    bool status = false;
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "GET /get HTTP/1.1\r\n";
    const auto host = port->info().value("remoteHost", "").toByteArray();
    if (host.isEmpty()) {
        throw sol::error(portName + ": Host not found");
    }
    txData += "Host: " + host + "\r\n";
    if (!headers.empty()) {
        const auto headersMap = uni_cast<QVariant>(sol::object(headers)).toMap();
        for (auto it = headersMap.constBegin(); it != headersMap.constEnd(); ++it) {
            const auto key = it.key().toUtf8();
            const auto value = it.value().toString().toUtf8();
            txData += key + ": " + value + "\r\n";
        }
    }

    QMetaObject::invokeMethod(port, [&status, &rxData, &port, &txData] {
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
