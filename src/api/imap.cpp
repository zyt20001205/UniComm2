#include "api/imap.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
Imap::Imap(QObject *parent)
    : QObject(parent) {
}

int Imap::idle(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    int exists{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray txData =
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

void Imap::login(const std::string &portName, const std::string &username, const std::string &password, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");
    m_count = 1;

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray txData =
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
    const QByteArray txData =
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

sol::object Imap::fetch(const sol::this_state ts, const std::string &portName, const int sequenceNumber, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QVariantHash parsed{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "FETCH"
            + ' '
            + QByteArray::number(sequenceNumber)
            + ' '
            + "BODY.PEEK[]";

    QMetaObject::invokeMethod(port, [&exception, &parsed, this, &port, &txData, &timeout] {
        QByteArray rxData{};
        QVariantHash session{};
        int size{};

        if (!port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            session = parser("FETCH", rxData);
            exception = session["exception"].toString();
            size = session["size"].toInt();
        }
        if (exception == "end") exception = "";

        rxData = port->read(size, timeout, "utf-8");
        parsed = fetchParser(rxData);
        // TODO: envelop () structure not supported!!! using read 3 to skip ")\r\n" for now
        rxData = port->read(3, timeout, "utf-8");

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
    return uni_cast<sol::object>(ts, parsed);
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
        if (command == "IDLE") return "failure: the server will not allow the IDLE command at this time";
        if (command == "LOGIN") return "login failure: user name or password rejected";
        if (command == "SELECT") return "select failure, now in authenticated state: no such mailbox, can't access mailbox";
        if (command == "FETCH") return "fetch error: can't fetch that data";
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
        if (command == "IDLE") {
            session["exception"] = "end";
            const auto name = QString::fromUtf8(rxData.mid(space2 + 1));
            const auto number = head.toInt();
            session[name] = number;
        } else if (command == "FETCH") {
            session["exception"] = "end";
            const auto openingBrace = rxData.indexOf('{');
            const auto closingBrace = rxData.indexOf('}');
            const auto size = rxData.mid(openingBrace + 1, closingBrace - openingBrace - 1).toInt();
            session["size"] = size;
        } else session["exception"] = "";
    }
    // status
    else if (head == "OK" || head == "NO") {
        const auto &status = head;
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

QVariantHash Imap::fetchParser(const QByteArray &rxData) {
    QVariantHash parsed{};
    const auto separator = rxData.indexOf("\r\n\r\n");
    // parse header
    QVariantHash headerHash{};
    auto headers = rxData.left(separator + 4);
    headers.replace("\r\n", "\n");
    QString key{};
    QVariant value{};
    for (const auto &header: headers.split('\n')) {
        // header line
        if (header.contains(':')) {
            if (!key.isEmpty()) {
                headerHash[key] = value.toString().toHtmlEscaped();
                key.clear();
                value.clear();
            }
            const auto comma = header.indexOf(':');
            key = header.left(comma);
            value = header.mid(comma + 2);
        }
        // continuation line
        else {
            value = value.toString() + " " + header.trimmed();
        }
    }
    if (!key.isEmpty()) {
        headerHash[key] = value.toString().toHtmlEscaped();
        key.clear();
        value.clear();
    }
    parsed["header"] = headerHash;
    // parse body
    QVariantList bodyList{};
    QVariantHash bodyHash{};
    const auto bodies = rxData.mid(separator + 4);
    qsizetype pos = 0;
    while (true) {
        const auto current = bodies.indexOf("\r\n\r\n", pos);
        if (current == -1) break;
        auto body = bodies.mid(pos, current - pos);
        // body header
        if (body.contains("Content-Type")) {
            body.replace("\r\n", "\n");
            for (const auto &header: body.split('\n')) {
                // header line
                if (header.contains(':')) {
                    if (!key.isEmpty()) {
                        bodyHash[key] = value.toString().toHtmlEscaped();
                        key.clear();
                        value.clear();
                    }
                    const auto comma = header.indexOf(':');
                    key = header.left(comma);
                    value = header.mid(comma + 2);
                }
                // continuation line
                else {
                    value = value.toString() + " " + header.trimmed();
                }
            }
            if (!key.isEmpty()) {
                bodyHash[key] = value.toString().toHtmlEscaped();
                key.clear();
                value.clear();
            }
        }
        // TODO: boundary is not recorded
        else if (body.contains("--")) {
        }
        // body data
        else {
            bodyHash["Data"] = QByteArray::fromBase64(body);
            if (bodyHash.contains("Content-Type")) bodyList.append(bodyHash);
        }
        pos = current + 4;
    }
    parsed["body"] = bodyList;
    return parsed;
}
