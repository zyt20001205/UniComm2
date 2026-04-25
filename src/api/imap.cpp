#include "api/imap.h"

#include <QDir>
#include <QElapsedTimer>
#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
Imap::Imap(QObject *parent)
    : QObject(parent) {
}

void Imap::init(const std::string &portName, const int timeout) {
    m_count = 1;
    const auto _portName = QString::fromStdString(portName);
    if (!g_port->m_portHash.contains(_portName)) throw sol::error(portName + " does not exist");
    m_portName = portName;
    m_timeout = timeout;
    m_port = g_port->m_portHash[_portName];

    QString exception{};
    QMetaObject::invokeMethod(m_port, [&exception, this] {
        if (!m_port->open()) {
            exception = "open failed";
            return;
        }

        QByteArray rxData{};

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            const auto session = parser("GREET", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

int Imap::_idle(const int timeout) {
    QString exception{};
    int sequenceNumber{};

    const QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "IDLE";

    QMetaObject::invokeMethod(m_port, [&exception, &sequenceNumber, this, &txData, &timeout] {
        QByteArray rxData{};
        QVariantHash session{};

        if (!m_port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") {
            sequenceNumber = session["EXISTS"].toInt();
            exception = "";
        } else return;

        if (!m_port->write("DONE", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    return sequenceNumber;
}

int Imap::idle(sol::optional<int> timeout) {
    return _idle(timeout.value_or(600000));
}

void Imap::login(const std::string &username, const std::string &password) {
    QString exception{};
    const QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "LOGIN"
            + ' '
            + QByteArray::fromStdString(username)
            + ' '
            + QByteArray::fromStdString(password);

    QMetaObject::invokeMethod(m_port, [&exception, this, &txData] {
        QByteArray rxData{};

        if (!m_port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            exception = parser("LOGIN", rxData)["exception"].toString();
        }
        if (exception == "end") exception = "";
        else return;
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Imap::select(const std::string &mailbox) {
    QString exception{};
    const QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "SELECT"
            + ' '
            + QByteArray::fromStdString(mailbox);

    QMetaObject::invokeMethod(m_port, [&exception, this, &txData] {
        QByteArray rxData{};

        if (!m_port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            const auto session = parser("SELECT", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

sol::object Imap::fetch(const sol::this_state ts, const int sequenceNumber) {
    QString exception{};
    QVariantHash parsed{};
    const QByteArray txData =
            'A'
            + QByteArray::number(m_count).rightJustified(3, '0')
            + ' '
            + "FETCH"
            + ' '
            + QByteArray::number(sequenceNumber)
            + ' '
            + "BODY.PEEK[]";

    QMetaObject::invokeMethod(m_port, [&exception, &parsed, this, &txData] {
        QByteArray rxData{};
        QVariantHash session{};
        int size{};

        if (!m_port->write(txData, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            session = parser("FETCH", rxData);
            exception = session["exception"].toString();
            size = session["size"].toInt();
        }
        if (exception == "end") exception = "";

        rxData = m_port->read(size, m_timeout, "utf-8");
        parsed = fetchParser(rxData);
        // TODO: envelop () structure not supported!!! using read 3 to skip ")\r\n" for now
        rxData = m_port->read(3, m_timeout, "utf-8");

        while (exception.isEmpty()) {
            rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            session = parser("IDLE", rxData);
            exception = session["exception"].toString();
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    return uni_cast<sol::object>(ts, parsed);
}

void Imap::_receive(const std::string &from, const std::string &path, const int timeout) {
    QString exception{};
    QVariantHash parsed{};

    QMetaObject::invokeMethod(m_port, [&exception, &parsed, this, &from, &timeout] {
        QElapsedTimer timer{};
        timer.start();

        QByteArray rxData{};
        QVariantHash session{};
        int sequenceNumber{};
        int size{};

        while (true) {
            // IDLE
            const QByteArray txData1 =
                    'A'
                    + QByteArray::number(m_count).rightJustified(3, '0')
                    + ' '
                    + "IDLE";

            if (!m_port->write(txData1, "utf-8", "crlf")) {
                exception = "write failed";
                return;
            }

            while (exception.isEmpty()) {
                rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
                session = parser("IDLE", rxData);
                exception = session["exception"].toString();
            }
            if (exception == "end") exception = "";
            else return;

            while (exception.isEmpty()) {
                const int remaining = timeout - static_cast<int>(timer.elapsed());
                qDebug() << "remaining" << remaining;
                rxData = m_port->readUntil("\r\n", remaining, "utf-8");
                session = parser("IDLE", rxData);
                exception = session["exception"].toString();
            }
            if (exception == "end") {
                sequenceNumber = session["EXISTS"].toInt();
                exception = "";
            } else return;

            if (!m_port->write("DONE", "utf-8", "crlf")) {
                exception = "write failed";
                return;
            }

            while (exception.isEmpty()) {
                rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
                session = parser("IDLE", rxData);
                exception = session["exception"].toString();
            }
            if (exception == "end") exception = "";
            else return;

            // FETCH
            const QByteArray txData2 =
                    'A'
                    + QByteArray::number(m_count).rightJustified(3, '0')
                    + ' '
                    + "FETCH"
                    + ' '
                    + QByteArray::number(sequenceNumber)
                    + ' '
                    + "BODY.PEEK[]";

            if (!m_port->write(txData2, "utf-8", "crlf")) {
                exception = "write failed";
                return;
            }

            while (exception.isEmpty()) {
                rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
                session = parser("FETCH", rxData);
                exception = session["exception"].toString();
                size = session["size"].toInt();
            }
            if (exception == "end") exception = "";
            else return;

            rxData = m_port->read(size, m_timeout, "utf-8");
            parsed = fetchParser(rxData);
            // TODO: envelop () structure not supported!!! using read 3 to skip ")\r\n" for now
            rxData = m_port->read(3, m_timeout, "utf-8");

            while (exception.isEmpty()) {
                rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
                session = parser("IDLE", rxData);
                exception = session["exception"].toString();
            }
            if (exception == "end") exception = "";
            else return;

            // check sender
            if (from.empty()) break;
            const auto _from = QString::fromStdString(from);
            const auto header = parsed["header"].toHash();
            if (header["From"].toString().contains(_from)) break;
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    const LPath luaPath = QString::fromStdString(path);
    const auto folderUrl = uni_cast<QUrl>(luaPath);
    const QDir dir(folderUrl.toLocalFile());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) throw sol::error("invalid path");
    }
    // save body
    const auto bodyList = parsed["body"].toList();
    for (const auto &value: bodyList) {
        const auto body = value.toHash();
        const auto type = body["Content-Type"].toString();
        QString fileName{};
        if (type.contains("text/plain")) {
            fileName = "body.txt";
        } else if (type.contains("text/html")) {
            fileName = "body.html";
        } else {
            emit appendLog(LOG_WARNING, "contact author:", QString("unsupported body (content-type:%1)").arg(type));
            continue;
        }
        QFile file(dir.filePath(fileName));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(body["Data"].toByteArray());
            file.close();
        }
    }
    // save attachment
    const auto attachmentList = parsed["attachment"].toList();
    for (const auto &value: attachmentList) {
        const auto attachment = value.toHash();
        const auto disposition = attachment["Content-Disposition"].toByteArray();
        // extract fileName
        const auto quote1 = disposition.indexOf('"', disposition.indexOf("filename"));
        if (quote1 == -1) continue;
        const auto quote2 = disposition.indexOf('"', quote1 + 1);
        if (quote2 == -1) continue;
        const auto fileName = rfc2047Parser(disposition.mid(quote1 + 1, quote2 - quote1 - 1));
        QFile file(dir.filePath(fileName));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(attachment["Data"].toByteArray());
            file.close();
        }
    }
}

void Imap::receive(const sol::optional<std::string> &from, const sol::optional<std::string> &path, sol::optional<int> timeout) {
    _receive(from.value_or(""), path.value_or(""), timeout.value_or(600000));
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
    QList<QByteArray> boundaries{{"\r\n\r\n"}};
    QVariantList bodyList{};
    QVariantList attachmentList{};
    qsizetype pos = 0;

    while (true) {
        qsizetype start{};
        qsizetype end{};
        QByteArray data = rxData;;
        QVariantHash hash{};
        QString key{};
        QVariant value{};

        bool isRoot = false;
        // match boundary
        if (boundaries.isEmpty()) break;
        const auto boundary = boundaries.last();
        // placeholder boundary
        if (boundary == "\r\n\r\n") {
            boundaries.pop_back();
        }
        // real boundary
        else {
            start = rxData.indexOf(boundary, pos);
            if (start == -1) break;
            end = rxData.indexOf("\r\n", start);
            if (end == -1) break;
            if (rxData.mid(start, end - start) == boundary + "--") {
                boundaries.pop_back();
                continue;
            }
            start = end + 2;
            end = rxData.indexOf(boundary, start);
            if (end == -1) break;
            data = rxData.mid(start, end - start);
        }
        // split data
        const auto separator = data.indexOf("\r\n\r\n");
        const auto headers = data.left(separator + 4).replace("\r\n", "\n");
        auto body = data.mid(separator + 4);
        // handle header
        for (const auto &header: headers.split('\n')) {
            // header line
            if (header.contains(':')) {
                if (!key.isEmpty()) {
                    // check isRoot
                    if (value.toByteArray().contains("boundary")) {
                        const auto _value = value.toByteArray();
                        const auto quote1 = _value.indexOf('"', _value.indexOf("boundary"));
                        if (quote1 == -1) continue;
                        const auto quote2 = _value.indexOf('"', quote1 + 1);
                        if (quote2 == -1) continue;
                        const auto _boundary = _value.mid(quote1 + 1, quote2 - quote1 - 1);
                        boundaries.append("--" + _boundary);
                        // mark as root
                        isRoot = true;
                    }
                    // insert pair
                    hash[key] = value.toString();
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
            // check isRoot
            if (value.toByteArray().contains("boundary")) {
                const auto _value = value.toByteArray();
                const auto _start = _value.indexOf("boundary");
                const auto quote1 = _value.indexOf('"', _start);
                if (quote1 == -1) continue;
                const auto quote2 = _value.indexOf('"', quote1 + 1);
                if (quote2 == -1) continue;
                const auto _boundary = _value.mid(quote1 + 1, quote2 - quote1 - 1);
                boundaries.append("--" + _boundary);
                // mark as root
                isRoot = true;
            }
            // insert pair
            hash[key] = value.toString();
            key.clear();
            value.clear();
        }
        // handle body
        if (!isRoot && !body.isEmpty()) {
            if (hash.value("Content-Transfer-Encoding").toByteArray().contains("base64")) {
                body = QByteArray::fromBase64(body);
            }
            hash["Data"] = body;
        }
        // insert hash
        if (hash.contains("Subject")) {
            parsed["header"] = hash;
        } else if (hash.contains("Content-Disposition")) {
            attachmentList.append(hash);
        } else if (hash.contains("Data")) {
            bodyList.append(hash);
        }
        pos = start;
    }

    parsed["body"] = bodyList;
    parsed["attachment"] = attachmentList;
    return parsed;
}

QByteArray Imap::rfc2047Parser(const QByteArray &text) {
    // =?charset?B?...?=  or =?charset?Q?...?=
    static const QRegularExpression re(R"(=\?([^?]+)\?([bBqQ])\?([^?]+)\?=)");
    const auto m = re.match(text);
    if (!m.hasMatch()) return text;

    const QByteArray charset = m.captured(1).toLatin1();
    const QByteArray encoding = m.captured(2).toLatin1().toUpper();
    const QByteArray payload = m.captured(3).toLatin1();

    if (encoding == 'B') {
        return QByteArray::fromBase64(payload);
    }
    // TODO: Q case
    emit appendLog(LOG_WARNING, "contact author:", QString("unsupported rfc2047 encoding (Q)"));
    return {};
}
