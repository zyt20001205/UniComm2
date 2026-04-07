#include "luaModule/imap.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

// public
Imap::Imap(QObject *parent)
    : QObject(parent) {
}

void Imap::login(const std::string &portName, const std::string &username, const std::string &password, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");
    m_count = 1;

    QByteArray rxData{};
    std::string exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "LOGIN";

    QMetaObject::invokeMethod(port, [&rxData, &exception, this, &port, &command, &timeout] {
        while (true) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parse(command, rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    bool status = false;
    rxData = {};
    exception = {};
    QByteArray txData = "A" + QByteArray::number(m_count++).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);
    txData.append(' ');
    txData.append(QByteArray::fromStdString(username));
    txData.append(' ');
    txData.append(QByteArray::fromStdString(password));
    QMetaObject::invokeMethod(port, [&status, &rxData, &exception, this, &port, &command, &txData, &timeout] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parse(command, rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);
}

// private
std::string Imap::parse(const QByteArray &command, const QByteArray &rxData) const {
    std::string exception = "contact author: unsupported imap response(" + std::string(rxData.constData(), rxData.length()) + ")";;
    const auto dataList = rxData.split(' ');
    for (int i = 0; i < dataList.size(); ++i) {
        const auto &data = dataList[i];
        switch (i) {
            case 0: {
                if (data.contains('A') && data.right(3).toInt() != m_count) exception = "tag mismatch";
            }
            break;
            case 1: {
                if (data == "OK") exception = "end";
                else if (data == "BAD") exception = "command unknown or arguments invalid";
                else if (data == "NO") {
                    if (command == "LOGIN") exception = "login failure: user name or password rejected";
                }
            }
            break;
            default: break;
        }
    }
    return exception;
}
