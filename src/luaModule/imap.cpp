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

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "LOGIN";
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);
    txData.append(' ');
    txData.append(QByteArray::fromStdString(username));
    txData.append(' ');
    txData.append(QByteArray::fromStdString(password));

    QMetaObject::invokeMethod(port, [&exception, this, &port, &command, &txData, &timeout] {
        QByteArray rxData{};

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parse(command, rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parse(command, rxData)["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void Imap::select(const std::string &portName, const std::string &mailbox, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "SELECT";
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);
    txData.append(' ');
    txData.append(QByteArray::fromStdString(mailbox));

    QMetaObject::invokeMethod(port, [&exception, this, &port, &command, &txData, &timeout] {
        QByteArray rxData{};

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parse(command, rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

int Imap::idle(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray command = "IDLE";
    QByteArray txData = "A" + QByteArray::number(m_count).rightJustified(3, '0');
    txData.append(' ');
    txData.append(command);

    QMetaObject::invokeMethod(port, [&exception, this, &port, &command, &txData, &timeout] {
        QByteArray rxData{};

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parse(command, rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parse(command, rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write("DONE", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parse(command, rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
    return true;
}

// private
QVariantHash Imap::parse(const QByteArray &command, const QByteArray &rxData) {
    QVariantHash session{};
    if (rxData.isEmpty()) {
        session["exception"] = "timeout";
        return session;
    }
    const auto dataList = rxData.split(' ');
    if (dataList.size() < 2) {
        session["exception"] = "invalid imap response";
        return session;
    }
    const auto &tag = dataList[0];
    const auto &status = dataList[1];
    if (tag.contains('A')) {
        if (tag.right(3).toInt() == m_count) {
            m_count++;
        } else {
            session["exception"] = "tag mismatch";
        }
        if (status == "OK") session["exception"] = "end";
        else if (status == "BAD") session["exception"] = "command unknown or arguments invalid";
        else if (status == "NO") {
            if (command == "LOGIN") session["exception"] = "login failure: user name or password rejected";
            if (command == "SELECT") session["exception"] = "select failure, now in authenticated state: no such mailbox, can't access mailbox";
            if (command == "IDLE") session["exception"] = "failure: the server will not allow the IDLE command at this time";
        }
    } else if (tag == '*') {
        if (command == "LOGIN") {
            if (status == "OK") session["exception"] = "end";
        } else if (command == "SELECT") {
        } else if (command == "IDLE") {
            if (dataList[2].contains("EXISTS")) session["exception"] = "end";
            else session["exception"] = "invalid imap response";
        } else {
            session["exception"] = "invalid imap response";
        }
    } else if (tag == '+') {
        if (command == "IDLE") {
            if (status.contains("idling")) session["exception"] = "end";
            else session["exception"] = "invalid imap response";
        } else {
            session["exception"] = "invalid imap response";
        }
    } else {
        session["exception"] = "invalid imap response";
    }
    return session;
}
