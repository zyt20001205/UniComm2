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

    bool status = false;
    rxData = {};
    exception = {};
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
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

void Imap::select(const std::string &portName, const std::string &mailbox, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    bool status = false;
    QByteArray rxData{};
    std::string exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "SELECT";
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);
    txData.append(' ');
    txData.append(QByteArray::fromStdString(mailbox));

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

bool Imap::idle(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    bool status = false;
    QByteArray rxData{};
    std::string exception{};
    bool exist = false;
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "IDLE";
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);

    QMetaObject::invokeMethod(port, [&status, &rxData, &exception, &exist, this, &port, &command, &txData, &timeout] {
        status = port->write(txData, "utf-8", "crlf");
        rxData = port->readUntil("\r\n", timeout, "utf-8");
        if (rxData != "+ idling\r\n") {
            exception = "invalid imap response";
            return;
        }
        rxData = port->readUntil("\r\n", timeout, "utf-8");
        if (!rxData.contains("EXISTS")) return;
        exist = true;
        status = port->write("DONE", "utf-8", "crlf");
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
    return exist;
}

// private
std::string Imap::parse(const QByteArray &command, const QByteArray &rxData) {
    std::string exception{};
    const auto dataList = rxData.split(' ');
    if (dataList.size() < 2) return "invalid imap response";
    const auto &tag = dataList[0];
    const auto &status = dataList[1];
    if (tag.contains('A')) {
        if (tag.right(3).toInt() == m_count) {
            m_count++;
        } else {
            exception = "tag mismatch";
        }
        if (status == "OK") {
            exception = "end";
        } else if (status == "BAD") {
            exception = "command unknown or arguments invalid";
        } else if (status == "NO") {
            if (command == "LOGIN") exception = "login failure: user name or password rejected";
            if (command == "SELECT") exception = "select failure, now in authenticated state: no such mailbox, can't access mailbox";
            if (command == "IDLE") exception = "failure: the server will not allow the IDLE command at this time";
        }
    } else {
        // qDebug() << rxData;
    }
    return exception;
}
