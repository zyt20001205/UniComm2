#include "api/imap.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QRegularExpression>
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
    const auto port = g_port->m_portHash.constFind(_portName);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
    m_portName = portName;
    m_timeout = timeout;
    m_port = port.value();

    QString exception{};

    QMetaObject::invokeMethod(m_port, [this]() -> QString {
        if (!m_port->open()) return "open failed";

        const auto rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
        const auto result = parser(rxData);
        if (!result.exception.isEmpty()) return result.exception;
        const auto untagged = std::get_if<Untagged>(&result.value);
        if (!untagged) return "invalid imap greeting";

        const auto space = untagged->value.indexOf(' ');
        if (space == -1) return "invalid imap greeting";
        const auto code = untagged->value.first(space);
        if (code == "OK" || code == "PREAUTH") return {};

        const auto text = QString::fromUtf8(untagged->value.sliced(space + 1));
        if (!text.isEmpty()) return text;
        return "unexpected imap response(" + QString::fromUtf8(code) + ")";
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Imap::login(const std::string &username, const std::string &password) {
    QString exception{};
    const auto _username = QByteArray::fromStdString(username);
    const auto _password = QByteArray::fromStdString(password);

    QMetaObject::invokeMethod(m_port, [this, &_username, &_password]() -> QString {
        const auto tag = nextTag();
        const auto txData = tag + " LOGIN " + _username + ' ' + _password;
        if (!m_port->write(txData, "utf-8", "crlf")) return "write failed";

        while (true) {
            const auto rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
            const auto result = parser(rxData);
            if (!result.exception.isEmpty()) return result.exception;

            if (const auto untagged = std::get_if<Untagged>(&result.value)) {
                const auto space = untagged->value.indexOf(' ');
                const auto code = space == -1 ? untagged->value : untagged->value.first(space);
                if (code != "BYE") continue;
                if (space != -1) return QString::fromUtf8(untagged->value.sliced(space + 1));
                return "server closed connection";
            }

            if (std::get_if<Continuation>(&result.value)) return "unexpected imap continuation";

            const auto tagged = std::get_if<Tagged>(&result.value);
            if (!tagged) return "invalid imap response";
            if (tagged->tag != tag) return "tag mismatch";
            if (tagged->code == "OK") return {};
            if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
            return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
        }
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

sol::object Imap::receive(const sol::this_state ts, const sol::optional<std::string> &from, const sol::optional<std::string> &path, const sol::optional<int> timeout) {
    QString exception{};
    QVariantHash parsed{};
    const auto _from = QString::fromStdString(from.value_or(""));
    const auto _path = path.value_or("");
    const auto _timeout = timeout.value_or(600000);

    QMetaObject::invokeMethod(m_port, [this, &parsed, &_from, _timeout]() -> QString {
        const auto selectTag = nextTag();
        if (!m_port->write(selectTag + " SELECT INBOX", "utf-8", "crlf")) return "write failed";

        while (true) {
            const auto result = parser(m_port->readUntil("\r\n", m_timeout, "utf-8"));
            if (!result.exception.isEmpty()) return result.exception;

            const auto tagged = std::get_if<Tagged>(&result.value);
            if (!tagged) continue;
            if (tagged->tag != selectTag) return "tag mismatch";
            if (tagged->code == "OK") break;
            if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
            return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
        }

        QElapsedTimer timer{};
        timer.start();

        while (true) {
            const auto idleTag = nextTag();
            if (!m_port->write(idleTag + " IDLE", "utf-8", "crlf")) return "write failed";

            while (true) {
                const auto result = parser(m_port->readUntil("\r\n", m_timeout, "utf-8"));
                if (!result.exception.isEmpty()) return result.exception;
                if (std::get_if<Continuation>(&result.value)) break;

                if (std::get_if<Untagged>(&result.value)) continue;

                if (const auto tagged = std::get_if<Tagged>(&result.value)) {
                    if (tagged->tag != idleTag) return "tag mismatch";
                    if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
                    return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
                }
            }

            int sequenceNumber{};
            QString waitException{};

            while (sequenceNumber == 0) {
                const auto remaining = _timeout - static_cast<int>(timer.elapsed());
                if (remaining <= 0) {
                    waitException = "read timeout";
                    break;
                }

                const auto result = parser(m_port->readUntil("\r\n", remaining, "utf-8"));
                if (!result.exception.isEmpty()) {
                    waitException = result.exception;
                    break;
                }

                if (const auto untagged = std::get_if<Untagged>(&result.value)) {
                    const auto space = untagged->value.indexOf(' ');
                    if (space == -1) continue;
                    bool numeric{};
                    const auto number = untagged->value.first(space).toInt(&numeric);
                    const auto tail = untagged->value.sliced(space + 1);
                    const auto nextSpace = tail.indexOf(' ');
                    const auto name = nextSpace == -1 ? tail : tail.first(nextSpace);
                    if (numeric && name == "EXISTS") sequenceNumber = number;
                    continue;
                }

                if (const auto tagged = std::get_if<Tagged>(&result.value)) {
                    if (tagged->tag != idleTag) return "tag mismatch";
                    if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
                    return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
                }
            }

            if (!m_port->write("DONE", "utf-8", "crlf")) return "write failed";

            while (true) {
                const auto result = parser(m_port->readUntil("\r\n", m_timeout, "utf-8"));
                if (!result.exception.isEmpty()) return result.exception;

                const auto tagged = std::get_if<Tagged>(&result.value);
                if (!tagged) continue;
                if (tagged->tag != idleTag) return "tag mismatch";
                if (tagged->code == "OK") break;
                if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
                return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
            }
            if (!waitException.isEmpty()) return waitException;

            const auto fetchTag = nextTag();
            if (!m_port->write(fetchTag + " FETCH " + QByteArray::number(sequenceNumber) + " BODY.PEEK[]", "utf-8", "crlf")) return "write failed";
            int size{-1};
            while (size < 0) {
                const auto result = parser(m_port->readUntil("\r\n", m_timeout, "utf-8"));
                if (!result.exception.isEmpty()) return result.exception;

                if (const auto untagged = std::get_if<Untagged>(&result.value)) {
                    const auto space = untagged->value.indexOf(' ');
                    if (space == -1) continue;
                    bool numeric{};
                    untagged->value.first(space).toInt(&numeric);
                    const auto tail = untagged->value.sliced(space + 1);
                    const auto nextSpace = tail.indexOf(' ');
                    const auto name = nextSpace == -1 ? tail : tail.first(nextSpace);
                    if (!numeric || name != "FETCH") continue;

                    const auto openingBrace = untagged->value.lastIndexOf('{');
                    const auto closingBrace = untagged->value.indexOf('}', openingBrace + 1);
                    if (openingBrace == -1 || closingBrace == -1) return "invalid imap fetch response";

                    bool validSize{};
                    size = untagged->value.sliced(openingBrace + 1, closingBrace - openingBrace - 1).toInt(&validSize);
                    if (!validSize) return "invalid imap fetch response";
                    continue;
                }

                if (const auto tagged = std::get_if<Tagged>(&result.value)) {
                    if (tagged->tag != fetchTag) return "tag mismatch";
                    if (tagged->code != "OK" && !tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
                    return "invalid imap fetch response";
                }
            }

            const auto rxData = m_port->read(size, m_timeout, "utf-8");
            if (rxData.size() != size) return "read timeout";
            if (m_port->readUntil("\r\n", m_timeout, "utf-8").isEmpty()) return "read timeout";

            while (true) {
                const auto result = parser(m_port->readUntil("\r\n", m_timeout, "utf-8"));
                if (!result.exception.isEmpty()) return result.exception;

                const auto tagged = std::get_if<Tagged>(&result.value);
                if (!tagged) continue;
                if (tagged->tag != fetchTag) return "tag mismatch";
                if (tagged->code == "OK") break;
                if (!tagged->text.isEmpty()) return QString::fromUtf8(tagged->text);
                return "unexpected imap response(" + QString::fromUtf8(tagged->code) + ")";
            }

            parsed = mailParser(rxData);
            if (_from.isEmpty()) break;
            if (parsed["header"].toHash()["From"].toString().contains(_from)) break;
        }

        return {};
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());

    const LPath luaPath = QString::fromStdString(_path);
    const auto folderUrl = uni_cast<QUrl>(luaPath);
    const QDir dir(folderUrl.toLocalFile());
    if (!dir.exists() && !dir.mkpath(".")) throw sol::error("invalid path");

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
            emit appendLog(LogLevel::Warning, "contact author:", QString("unsupported body (content-type:%1)").arg(type));
            continue;
        }

        QFile file(dir.filePath(fileName));
        if (file.open(QIODevice::WriteOnly)) file.write(body["Data"].toByteArray());
    }

    const auto attachmentList = parsed["attachment"].toList();
    for (const auto &value: attachmentList) {
        const auto attachment = value.toHash();
        const auto disposition = attachment["Content-Disposition"].toByteArray();
        const auto quote1 = disposition.indexOf('"', disposition.indexOf("filename"));
        if (quote1 == -1) continue;
        const auto quote2 = disposition.indexOf('"', quote1 + 1);
        if (quote2 == -1) continue;

        const auto fileName = rfc2047Parser(disposition.sliced(quote1 + 1, quote2 - quote1 - 1));
        QFile file(dir.filePath(fileName));
        if (file.open(QIODevice::WriteOnly)) file.write(attachment["Data"].toByteArray());
    }

    return uni_cast<sol::object>(ts, parsed["header"].toHash());
}

void Imap::logout() {
    QString exception{};

    QMetaObject::invokeMethod(m_port, [this]() -> QString {
        const auto tag = nextTag();
        if (!m_port->write(tag + " LOGOUT", "utf-8", "crlf")) return "write failed";

        return {};
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

// private
Imap::Result Imap::parser(const QByteArray &rxData) {
    if (rxData.isEmpty()) return {std::monostate{}, "read timeout"};

    const auto response = rxData.trimmed();
    // Continuation
    if (response.startsWith('+')) return {Continuation{response.sliced(1).trimmed()}, {}};
    // Untagged
    if (response.startsWith('*')) return {Untagged{response.sliced(1).trimmed()}, {}};
    // Tagged
    const auto space1 = response.indexOf(' ');
    if (space1 == -1) return {std::monostate{}, "invalid imap tagged response"};
    const auto space2 = response.indexOf(' ', space1 + 1);
    if (space2 == -1) return {std::monostate{}, "invalid imap tagged response"};
    return {Tagged{response.first(space1), response.sliced(space1 + 1, space2 - space1 - 1), response.sliced(space2 + 1)}, {}};
}

QByteArray Imap::nextTag() {
    return 'A' + QByteArray::number(m_count++).rightJustified(3, '0');
}

QVariantHash Imap::mailParser(const QByteArray &rxData) {
    QVariantHash parsed{};
    QList<QByteArray> boundaries{{"\r\n\r\n"}};
    QVariantList bodyList{};
    QVariantList attachmentList{};
    qsizetype pos = 0;
    bool first = true;

    while (true) {
        qsizetype start{};
        qsizetype end{};
        QByteArray data = rxData;
        QVariantHash hash{};
        QString key{};
        QVariant value{};

        bool isRoot = false;
        if (boundaries.isEmpty()) break;
        const auto boundary = boundaries.last();
        if (boundary == "\r\n\r\n") {
            boundaries.pop_back();
        } else {
            start = rxData.indexOf(boundary, pos);
            if (start == -1) break;
            end = rxData.indexOf("\r\n", start);
            if (end == -1) break;
            if (rxData.sliced(start, end - start) == boundary + "--") {
                boundaries.pop_back();
                continue;
            }
            start = end + 2;
            end = rxData.indexOf(boundary, start);
            if (end == -1) break;
            data = rxData.sliced(start, end - start);
        }

        const auto separator = data.indexOf("\r\n\r\n");
        const auto headers = data.first(separator + 4).replace("\r\n", "\n");
        auto body = data.sliced(separator + 4);

        for (const auto &header: headers.split('\n')) {
            if (header.contains(':')) {
                if (!key.isEmpty()) {
                    if (value.toByteArray().contains("boundary")) {
                        const auto _value = value.toByteArray();
                        const auto quote1 = _value.indexOf('"', _value.indexOf("boundary"));
                        if (quote1 == -1) continue;
                        const auto quote2 = _value.indexOf('"', quote1 + 1);
                        if (quote2 == -1) continue;
                        boundaries.append("--" + _value.sliced(quote1 + 1, quote2 - quote1 - 1));
                        isRoot = true;
                    }
                    hash[key] = value.toString();
                    key.clear();
                    value.clear();
                }
                const auto comma = header.indexOf(':');
                key = header.first(comma);
                value = header.sliced(comma + 2);
            } else {
                value = value.toString() + " " + header.trimmed();
            }
        }

        if (!key.isEmpty()) {
            if (value.toByteArray().contains("boundary")) {
                const auto _value = value.toByteArray();
                const auto quote1 = _value.indexOf('"', _value.indexOf("boundary"));
                if (quote1 == -1) continue;
                const auto quote2 = _value.indexOf('"', quote1 + 1);
                if (quote2 == -1) continue;
                boundaries.append("--" + _value.sliced(quote1 + 1, quote2 - quote1 - 1));
                isRoot = true;
            }
            hash[key] = value.toString();
        }

        if (!isRoot && !body.isEmpty()) {
            if (hash.value("Content-Transfer-Encoding").toByteArray().contains("base64")) {
                body = QByteArray::fromBase64(body);
            }
            hash["Data"] = body;
        }

        if (first) {
            auto header = hash;
            header.remove("Data");
            parsed["header"] = header;
            first = false;
        }

        if (hash.contains("Content-Disposition")) {
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
    static const QRegularExpression re(R"(=\?([^?]+)\?([bBqQ])\?([^?]+)\?=)");
    const auto m = re.match(text);
    if (!m.hasMatch()) return text;

    const QByteArray encoding = m.captured(2).toLatin1().toUpper();
    const QByteArray payload = m.captured(3).toLatin1();

    if (encoding == 'B') return QByteArray::fromBase64(payload);
    emit appendLog(LogLevel::Warning, "contact author:", QString("unsupported rfc2047 encoding (Q)"));
    return {};
}
