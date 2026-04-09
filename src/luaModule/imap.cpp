#include "luaModule/imap.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/uniCast.h"

// public
Imap::Imap(QObject *parent)
    : QObject(parent) {
}

void Imap::login(const std::string &portName, const std::string &username, const std::string &password, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");
    m_count = 1;

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "LOGIN"
            + ' '
            + QByteArray::fromStdString(username)
            + ' '
            + QByteArray::fromStdString(password);

    QMetaObject::invokeMethod(port, [&exception, this, &port, &txData, &timeout] {
        QByteArray rxData{};

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parser("GREET", rxData);
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
            exception = parser("LOGIN", rxData)["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void Imap::select(const std::string &portName, const std::string &mailbox, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "SELECT"
            + ' '
            + QByteArray::fromStdString(mailbox);

    QMetaObject::invokeMethod(port, [&exception, this, &port, &txData, &timeout] {
        QByteArray rxData{};

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            const auto session = parser("SELECT", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

sol::table Imap::fetch(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "UID SEARCH";

    qDebug() << txData;
    return {};
}

int Imap::idle(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    int exists{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "IDLE";

    QMetaObject::invokeMethod(port, [&exception, &exists, this, &port, &txData, &timeout] {
        QByteArray rxData{};
        QVariantHash session{};

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") {
            exists = session["EXISTS"].toInt();
            exception = "";
        } else return;

        if (!port->write("DONE", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
    return exists;
}

// private
QVariantHash Imap::parser(const QByteArray &command, const QByteArray &rxData) {
    QVariantHash session{};
    session["exception"] = "contact author: unsupported tagged response(" + QString::fromUtf8(rxData) + ")";
    if (rxData.isEmpty()) {
        session["exception"] = "read timeout";
        return session;
    }
    if (rxData.startsWith('+')) {
        session["exception"] = continuationParser(command, rxData.trimmed());
        return session;
    }
    if (rxData.startsWith('A')) {
        session["exception"] = taggedParser(command, rxData.trimmed());
        return session;
    }
    if (rxData.startsWith('*')) {
        return untaggedParser(command, rxData.trimmed());
    }
    return session;
}

QString Imap::continuationParser(const QByteArray &command, const QByteArray &rxData) {
    if (command == "IDLE" && rxData == "+ idling") return "end";
    return "contact author: unsupported tagged response(" + QString::fromUtf8(rxData) + ")";
}

QString Imap::taggedParser(const QByteArray &command, const QByteArray &rxData) {
    const auto space1 = rxData.indexOf(' ');
    if (space1 == -1) return "invalid imap tagged response";
    const auto space2 = rxData.indexOf(' ', space1 + 1);
    if (space2 == -1) return "invalid imap tagged response";
    const auto tag = rxData.left(space1);
    const auto status = rxData.mid(space1 + 1, space2 - space1 - 1);
    // TODO: split resp
    // const auto text = rxData.mid(space2 + 1);
    // qDebug() << text;

    if (tag.right(3).toInt() == m_count) m_count++;
    else return "tag mismatch";

    if (status == "OK") return "end";
    if (status == "BAD") return "command unknown or arguments invalid";
    if (status == "NO") {
        if (command == "LOGIN") return "login failure: user name or password rejected";
        if (command == "SELECT") return "select failure, now in authenticated state: no such mailbox, can't access mailbox";
        if (command == "IDLE") return "failure: the server will not allow the IDLE command at this time";
    }

    return "contact author: unsupported tagged response(" + QString::fromUtf8(rxData) + ")";
}

QVariantHash Imap::untaggedParser(const QByteArray &command, const QByteArray &rxData) {
    QVariantHash session{};
    session["exception"] = "contact author: unsupported tagged response(" + QString::fromUtf8(rxData) + ")";
    const auto space1 = rxData.indexOf(' ');
    if (space1 == -1) {
        session["exception"] = "invalid imap response";
        return session;
    }
    const auto space2 = rxData.indexOf(' ', space1 + 1);
    if (space2 == -1) {
        session["exception"] = "invalid imap response";
        return session;
    }
    const auto head = rxData.mid(space1 + 1, space2 - space1 - 1);

    // numeric
    if (head[0] >= '0' && head[0] <= '9') {
        if (command == "IDLE") session["exception"] = "end";
        else session["exception"] = "";
        const auto name = QString::fromUtf8(rxData.mid(space2 + 1));
        const auto number = head.toInt();
        session[name] = number;
    }
    // status
    else if (head == "OK" || head == "NO") {
        const auto& status = head;
        // TODO: split resp
        // const auto text = rxData.mid(space2 + 1);
        // qDebug() << text;
        if (status == "OK") {
            if (command == "GREET") session["exception"] = "end";
            else session["exception"] = "";
        } else {
        }
    }
    // data
    else if (head == "FLAGS") {
        session["exception"] = "";
    }
    return session;
}
