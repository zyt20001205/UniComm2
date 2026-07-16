#include "api/http.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
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

    QByteArray remoteHost{};
    QMetaObject::invokeMethod(m_port, [&remoteHost, this] {
        remoteHost = m_port->config().value("remoteHost").toString().toUtf8();
    }, Qt::BlockingQueuedConnection);
    m_remoteHost = remoteHost;
}

sol::object Http::del(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header, const sol::optional<std::string> &body) const {
    return request(ts, "DELETE", target, header, body);
}

sol::object Http::get(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header) const {
    return request(ts, "GET", target, header, {});
}

sol::object Http::head(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header) const {
    return request(ts, "HEAD", target, header, {});
}

sol::object Http::patch(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header, const sol::optional<std::string> &body) const {
    return request(ts, "PATCH", target, header, body);
}

sol::object Http::post(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header, const sol::optional<std::string> &body) const {
    return request(ts, "POST", target, header, body);
}

sol::object Http::put(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header, const sol::optional<std::string> &body) const {
    return request(ts, "PUT", target, header, body);
}

// private
sol::object Http::request(const sol::this_state ts, const QByteArray &method, const std::string &target, const sol::optional<sol::table> &header,
                          const sol::optional<std::string> &body) const {
    QString exception{};
    QVariantHash parsed{};

    QByteArray txData = method + " " + QByteArray::fromStdString(target) + " HTTP/1.1\r\n";
    txData += "Host: " + m_remoteHost + "\r\n";
    const auto _body = body.has_value() ? QByteArray::fromStdString(body.value()) : QByteArray{};
    if (body.has_value()) txData += "Content-Length: " + QByteArray::number(_body.size()) + "\r\n";
    if (header.has_value()) {
        for (const auto &[key, value]: header.value()) {
            if (!key.is<std::string>() || !value.is<std::string>()) continue;
            const auto name = QByteArray::fromStdString(key.as<std::string>());
            const auto fieldValue = QByteArray::fromStdString(value.as<std::string>());
            if (name.compare("Host", Qt::CaseInsensitive) == 0 ||
                name.compare("Content-Length", Qt::CaseInsensitive) == 0 ||
                name.compare("Transfer-Encoding", Qt::CaseInsensitive) == 0)
                continue;
            txData += name + ": " + fieldValue + "\r\n";
        }
    }
    txData += "\r\n";
    if (body.has_value()) txData += _body;

    QMetaObject::invokeMethod(m_port, [&exception, &parsed, this, &method, &txData] {
        if (!m_port->open()) {
            exception = "open failed";
            return;
        }

        if (!m_port->write(txData, "utf-8", "null")) {
            exception = "write failed";
            return;
        }

        const QByteArray rxHeader = m_port->readUntil("\r\n\r\n", m_timeout, "utf-8");
        if (rxHeader.isEmpty()) {
            exception = "read timeout";
            return;
        }

        parsed = parser(rxHeader);
        exception = parsed.take("exception").toString();
        if (!exception.isEmpty() || method == "HEAD") return;

        const auto responseHeader = parsed.value("header").toHash();
        QByteArray rxBody{};

        if (responseHeader.value("transfer-encoding").toString().contains("chunked", Qt::CaseInsensitive)) {
            while (true) {
                // size line
                auto sizeLine = m_port->readUntil("\r\n", m_timeout, "utf-8");
                if (sizeLine.isEmpty()) {
                    exception = "read timeout";
                    return;
                }
                // remove eol
                sizeLine.chop(2);
                // remove trailer: "1000;some-extension=value\r\n"
                const auto semicolon = sizeLine.indexOf(';');
                if (semicolon != -1) sizeLine.truncate(semicolon);
                // get chunk size
                bool ok{};
                const auto chunkSize = sizeLine.trimmed().toInt(&ok, 16);
                if (!ok || chunkSize < 0) {
                    exception = "invalid HTTP chunk size";
                    return;
                }

                // dump trailer
                if (chunkSize == 0) {
                    while (true) {
                        const auto trailer = m_port->readUntil("\r\n", m_timeout, "utf-8");
                        if (trailer.isEmpty()) {
                            exception = "read timeout";
                            return;
                        }
                        if (trailer == "\r\n") break;
                    }
                    break;
                }
                // read chunk
                const auto data = m_port->read(chunkSize, m_timeout, "utf-8");
                if (data.size() != chunkSize) {
                    exception = "read timeout";
                    return;
                }
                rxBody += data;
                // dump delimiter
                if (m_port->read(2, m_timeout, "utf-8") != "\r\n") {
                    exception = "invalid HTTP chunk delimiter";
                    return;
                }
            }
        } else {
            const int contentLength = responseHeader.value("content-length").toInt();
            if (contentLength > 0) {
                rxBody = m_port->read(contentLength, m_timeout, "utf-8");
                if (rxBody.size() != contentLength) {
                    exception = "read timeout";
                    return;
                }
            }
        }
        parsed["body"] = rxBody;
    }, Qt::BlockingQueuedConnection);

    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    return uni_cast<sol::object>(ts, parsed);
}

QVariantHash Http::parser(const QByteArray &rxData) {
    // status line
    const auto eol = rxData.indexOf("\r\n");
    if (eol == -1) return {{"exception", "invalid HTTP status line"}};
    const auto line = rxData.first(eol);
    const auto space = line.indexOf(' ');
    if (space == -1) return {{"exception", "invalid HTTP status line"}};
    const auto version = QString::fromUtf8(line.first(space));
    const auto statusCode = line.mid(space + 1, 3).toInt();
    const auto reason = QString::fromUtf8(line.mid(space + 5));

    // header
    QVariantHash header{};
    qsizetype pos = eol + 2;
    while (pos < rxData.size() - 2) {
        const auto _eol = rxData.indexOf("\r\n", pos);
        if (_eol == pos) break;

        const auto _line = rxData.mid(pos, _eol - pos);
        const auto colon = _line.indexOf(':');
        if (colon > 0) {
            const auto name = _line.first(colon);
            const auto key = QString::fromUtf8(name).toLower();
            const auto value = QString::fromUtf8(_line.mid(colon + 1).trimmed());
            header[key] = value;
        }

        pos = _eol + 2;
    }

    return {
        {"version", version},
        {"statusCode", statusCode},
        {"reason", reason},
        {"header", header},
        {"body", QByteArray{}}
    };
}
