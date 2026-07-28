#include "api/ftp.h"

#include <QList>
#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"

// public
Ftp::Ftp(QObject *parent)
    : QObject(parent) {
}

void Ftp::init(const std::string &portName, const int timeout) {
    const auto name = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(name);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");

    m_portName = portName;
    m_timeout = timeout;
    m_port = port.value();

    QString exception{};

    QMetaObject::invokeMethod(m_port, [this]() -> QString {
        if (!m_port->open()) return "open failed";

        while (true) {
            const auto result = response();
            if (!result.exception.isEmpty()) return result.exception;
            if (result.code == StatusCode::ServiceReadyInMinutes) continue;
            if (result.code == StatusCode::ServiceReady) return {};
            return "unexpected ftp response(" + QString::number(result.code) + ")";
        }
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Ftp::login(const std::string &username, const std::string &password) const {
    QString exception{};
    const auto _username = QByteArray::fromStdString(username);
    const auto _password = QByteArray::fromStdString(password);

    QMetaObject::invokeMethod(m_port, [this, &_username, &_password]() -> QString {
        if (!m_port->write("USER " + _username, "utf-8", "crlf")) return "write failed";

        auto result = response();
        if (!result.exception.isEmpty()) return result.exception;
        if (result.code == StatusCode::UserLoggedIn) return {};
        if (result.code != StatusCode::UserNameOkay) return "unexpected ftp response(" + QString::number(result.code) + ")";

        if (!m_port->write("PASS " + _password, "utf-8", "crlf")) return "write failed";

        result = response();
        if (!result.exception.isEmpty()) return result.exception;
        if (result.code != StatusCode::UserLoggedIn) return "unexpected ftp response(" + QString::number(result.code) + ")";

        return {};
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Ftp::quit() const {
    QString exception{};

    QMetaObject::invokeMethod(m_port, [this]() -> QString {
        if (!m_port->write("QUIT", "utf-8", "crlf")) return "write failed";

        return {};
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

// private
Ftp::Result Ftp::response() const {
    QByteArray rxData = m_port->readUntil("\r\n", m_timeout, "utf-8");
    if (rxData.isEmpty()) return {.exception = "read timeout"};

    if (rxData.size() >= 6 && rxData.at(3) == '-') {
        const auto terminator = rxData.first(3) + ' ';
        while (true) {
            const auto line = m_port->readUntil("\r\n", m_timeout, "utf-8");
            if (line.isEmpty()) return {.exception = "read timeout"};
            rxData += line;
            if (line.startsWith(terminator)) break;
        }
    }

    return parser(rxData);
}

Ftp::Result Ftp::parser(const QByteArray &rxData) {
    if (rxData.isEmpty()) return {.exception = "read timeout"};

    const auto firstEol = rxData.indexOf("\r\n");
    if (firstEol == -1) return {.exception = "invalid ftp response"};
    const auto firstLine = rxData.first(firstEol);
    if (firstLine.size() < 4) return {.exception = "invalid ftp response"};

    const int code = firstLine.first(3).toInt();
    if (code < 100 || code >= 600) return {.exception = "invalid ftp response"};

    const char separator = firstLine.at(3);
    if (separator != ' ' && separator != '-') return {.exception = "invalid ftp response"};

    QList<QByteArray> lines{};
    lines.append(firstLine.sliced(4));

    if (separator == '-') {
        const auto terminator = firstLine.first(3) + ' ';
        qsizetype pos = firstEol + 2;
        bool terminated{};

        while (pos < rxData.size()) {
            const auto eol = rxData.indexOf("\r\n", pos);
            if (eol == -1) return {.exception = "invalid ftp response"};
            const auto line = rxData.sliced(pos, eol - pos);
            if (line.startsWith(terminator)) {
                lines.append(line.sliced(4));
                terminated = true;
                break;
            }
            lines.append(line);
            pos = eol + 2;
        }

        if (!terminated) return {.exception = "invalid ftp response"};
    }

    const auto text = QString::fromUtf8(lines.join('\n').trimmed());
    if (code >= 400) {
        if (!text.isEmpty()) return {code, text, text};
        return {code, {}, "ftp error(" + QString::number(code) + ")"};
    }
    return {code, text, {}};
}
