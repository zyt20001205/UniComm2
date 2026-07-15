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
}

sol::object Http::head(const sol::this_state ts, const std::string &target, const sol::optional<sol::table> &header) const {
    const QByteArray _target(target.data(), static_cast<qsizetype>(target.size()));
    if (_target.contains('\r') || _target.contains('\n')) throw sol::error("invalid HTTP target");

    QByteArray _header{};
    if (header.has_value()) {
        const auto headerMap = uni_cast<QVariant>(sol::object(header.value())).toMap();
        for (auto it = headerMap.constBegin(); it != headerMap.constEnd(); ++it) {
            const QByteArray name = it.key().toUtf8();
            const QByteArray value = it.value().toString().toUtf8();
            if (name.compare("Host", Qt::CaseInsensitive) == 0) continue;
            _header += name + ": " + value + "\r\n";
        }
    }

    QString exception{};
    QVariantHash parsed{};
    QMetaObject::invokeMethod(m_port, [&exception, &parsed, this, &_target, &_header] {
        if (!m_port->open()) {
            exception = "open failed";
            return;
        }

        QByteArray txData = "HEAD " + _target + " HTTP/1.1\r\n";
        txData += "Host: " + m_port->config().value("remoteHost").toString().toUtf8() + "\r\n";
        txData += _header + "\r\n";

        if (!m_port->write(txData, "utf-8", "null")) {
            exception = "write failed";
            return;
        }

        const QByteArray rxData = m_port->readUntil("\r\n\r\n", m_timeout, "utf-8");
        if (rxData.isEmpty()) {
            exception = "read timeout";
            return;
        }

        parsed = headerParser(rxData);
        exception = parsed.take("exception").toString();
    }, Qt::BlockingQueuedConnection);

    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    return uni_cast<sol::object>(ts, parsed);
}

sol::object Http::post(const sol::this_state ts, const std::string &target, const std::string &body, const sol::optional<sol::table> &header) const {
    const QByteArray _target(target.data(), static_cast<qsizetype>(target.size()));
    if (_target.contains('\r') || _target.contains('\n')) throw sol::error("invalid HTTP target");
    const QByteArray _body(body.data(), static_cast<qsizetype>(body.size()));

    QByteArray _header{};
    if (header.has_value()) {
        const auto headerMap = uni_cast<QVariant>(sol::object(header.value())).toMap();
        for (auto it = headerMap.constBegin(); it != headerMap.constEnd(); ++it) {
            const QByteArray name = it.key().toUtf8();
            const QByteArray value = it.value().toString().toUtf8();
            if (name.compare("Host", Qt::CaseInsensitive) == 0 || name.compare("Content-Length", Qt::CaseInsensitive) == 0) continue;
            _header += name + ": " + value + "\r\n";
        }
    }

    QString exception{};
    QVariantHash parsed{};
    QMetaObject::invokeMethod(m_port, [&exception, &parsed, this, &_target, &_body, &_header] {
        if (!m_port->open()) {
            exception = "open failed";
            return;
        }

        QByteArray txData = "POST " + _target + " HTTP/1.1\r\n";
        txData += "Host: " + m_port->config().value("remoteHost").toString().toUtf8() + "\r\n";
        txData += "Content-Length: " + QByteArray::number(_body.size()) + "\r\n";
        txData += _header + "\r\n" + _body;

        if (!m_port->write(txData, "utf-8", "null")) {
            exception = "write failed";
            return;
        }

        const QByteArray rxHeader = m_port->readUntil("\r\n\r\n", m_timeout, "utf-8");
        if (rxHeader.isEmpty()) {
            exception = "read timeout";
            return;
        }

        parsed = headerParser(rxHeader);
        exception = parsed.take("exception").toString();
        if (!exception.isEmpty()) return;

        const auto header = parsed.value("header").toHash();
        const auto contentLength = header.value("content-length").toInt();
        QByteArray rxBody{};
        if (contentLength > 0) {
            rxBody = m_port->read(contentLength, m_timeout, "utf-8");
            if (rxBody.isEmpty()) {
                exception = "read timeout";
                return;
            }
        }
        // TODO: Handle Transfer-Encoding: chunked.
        parsed["body"] = rxBody;
    }, Qt::BlockingQueuedConnection);

    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
    return uni_cast<sol::object>(ts, parsed);
}

// void Http::get(const std::string &portName, const sol::table &header) {
//     const auto portIt = g_port->m_portHash.constFind(QString::fromStdString(portName));
//     if (portIt == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
//
//     bool status = false;
//     QByteArray rxData{};
//     auto *port = portIt.value();
//     QByteArray txData = "GET /get HTTP/1.1\r\n";
//     const auto host = port->info().value("remoteHost", "").toByteArray();
//     if (host.isEmpty()) {
//         throw sol::error(portName + ": Host not found");
//     }
//     txData += "Host: " + host + "\r\n";
//     if (!header.empty()) {
//         const auto headerMap = uni_cast<QVariant>(sol::object(header)).toMap();
//         for (auto it = headerMap.constBegin(); it != headerMap.constEnd(); ++it) {
//             const auto key = it.key().toUtf8();
//             const auto value = it.value().toString().toUtf8();
//             txData += key + ": " + value + "\r\n";
//         }
//     }
//
//     QMetaObject::invokeMethod(port, [&status, &rxData, &port, &txData] {
//         status = port->write(txData, "ascii", "crlf");
//         // port->clear();
//         // rxData = port->read(3, 1000, "ascii");
//     }, Qt::BlockingQueuedConnection);
//     // if (!status || rxData.isEmpty()) {
//     //     throw sol::error(portName + ": communication failed");
//     // }
//     // if (!QString::fromLatin1(rxData).startsWith("250")) {
//     //     throw sol::error(portName + ": SMTP EHLO failed");
//     // }
// }

// private
QVariantHash Http::headerParser(const QByteArray &rxData) {
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
