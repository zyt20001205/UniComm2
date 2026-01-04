#include "luaModule/luaSMTP.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

LuaSMTP::LuaSMTP(QObject *parent)
    : QObject(parent) {
}

void LuaSMTP::ehlo(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "EHLO localhost";
    bool status = false;

    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP EHLO failed");
    }
}

void LuaSMTP::authLogin(const std::string &portName, const std::string &username, const std::string &password) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "AUTH LOGIN";
    bool status = false;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("334")) {
        throw sol::error(portName + ": SMTP AUTH LOGIN failed");
    }

    txData = QByteArray::fromStdString(username).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("334")) {
        throw sol::error(portName + ": SMTP invalid username");
    }

    txData = QByteArray::fromStdString(password).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("235")) {
        throw sol::error(portName + ": SMTP invalid password");
    }
}

void LuaSMTP::mail(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "MAIL FROM: <" + QByteArray::fromStdString(from) + ">";
    bool status = false;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP MAIL FROM failed");
    }

    txData = "RCPT TO: <" + QByteArray::fromStdString(to) + ">";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP RCPT failed");
    }

    txData = "DATA";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("354")) {
        throw sol::error(portName + ": SMTP DATA failed");
    }

    txData = "From: " + QByteArray::fromStdString(from) + "\r\n";
    txData += "To: " + QByteArray::fromStdString(to) + "\r\n";
    txData += "Subject: " + QByteArray::fromStdString(subject) + "\r\n\r\n";
    txData += QByteArray::fromStdString(body) + "\r\n.";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        rxData = port->read(1000, 3, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP MAIL failed");
    }

    txData = "QUIT";
    status = false;

    QMetaObject::invokeMethod(port, [&port, &txData, &status] {
        status = port->write(txData, "ascii", "crlf");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
}
