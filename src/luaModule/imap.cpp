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
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");
    m_count = 1;

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "LOGIN";
    QByteArray rxData{};
    std::string exception{};

    QMetaObject::invokeMethod(port, [&port, &command, &rxData, &exception] {
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(command, rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    QByteArray txData = "A" + QByteArray::number(m_count++).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);
    txData.append(' ');
    txData.append(QByteArray::fromStdString(username));
    txData.append(' ');
    txData.append(QByteArray::fromStdString(password));
    bool status = false;
    rxData = {};
    QMetaObject::invokeMethod(port, [&port, &command, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
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
std::string Imap::parse(const QByteArray &command, const QByteArray &rxData) {
    std::string exception = "contact author: unsupported imap response(" + std::string(rxData.constData(), rxData.length()) + ")";;
    const auto dataList = rxData.split(' ');
    for (int i = 0; i < dataList.size(); ++i) {
        const auto &data = dataList[i];
        switch (i) {
            case 0: {
            }
            break;
            case 1: {
                if (data == "OK") exception = "end";
                if (data == "BAD") exception = "command unknown or arguments invalid";
                if (data == "NO") {
                    if (command == "LOGIN") exception = "login failure: user name or password rejected";
                }
            }
            break;
            default: break;
        }
    }
    return exception;
}
