#include "api/ftp.h"

#include <QDeadlineTimer>
#include <QList>
#include <QTcpSocket>
#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
Ftp::Ftp(QObject *parent)
    : QObject(parent) {
}

void Ftp::init(const std::string &portName, const int timeout) {
    const auto name = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(name);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
    if (port.value()->type() != PortType::TcpClient) throw sol::error(portName + " is not a TCP client");

    m_portName = portName;
    m_timeout = timeout;
    m_port = port.value();

    QString exception{};

    QMetaObject::invokeMethod(m_port, [this]() -> QString {
        if (!m_port->open()) return "open failed";

        while (true) {
            const auto result = ctrlResponse();
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

        auto result = ctrlResponse();
        if (!result.exception.isEmpty()) return result.exception;
        if (result.code == StatusCode::UserLoggedIn) return {};
        if (result.code != StatusCode::UserNameOkay) return "unexpected ftp response(" + QString::number(result.code) + ")";

        if (!m_port->write("PASS " + _password, "utf-8", "crlf")) return "write failed";

        result = ctrlResponse();
        if (!result.exception.isEmpty()) return result.exception;
        if (result.code != StatusCode::UserLoggedIn) return "unexpected ftp response(" + QString::number(result.code) + ")";

        return {};
    }, Qt::BlockingQueuedConnection, &exception);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

sol::table Ftp::list(const sol::this_state ts, const sol::optional<std::string> &path) const {
    DataResult result{};
    const auto _path = QByteArray::fromStdString(path.value_or(""));

    QMetaObject::invokeMethod(m_port, [this, &_path, &result]() {
        QByteArray command{"MLSD"};
        if (!_path.isEmpty()) command += ' ' + _path;
        result = dataResponse(command);
    }, Qt::BlockingQueuedConnection);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    QVariantList entries{};
    for (auto line: result.data.split('\n')) {
        if (line.endsWith('\r')) line.chop(1);
        if (line.isEmpty()) continue;

        const auto space = line.indexOf(' ');
        if (space <= 0 || space == line.size() - 1)
            throw sol::error(m_portName + ": invalid MLSD response");

        QByteArray type{};
        QByteArray size{};
        QByteArray modified{};

        for (const auto &fact: line.first(space).split(';')) {
            if (fact.isEmpty()) continue;
            const auto equal = fact.indexOf('=');
            if (equal <= 0) throw sol::error(m_portName + ": invalid MLSD response");

            const auto key = fact.first(equal).toLower();
            const auto value = fact.sliced(equal + 1);
            if (key == "type") type = value.toLower();
            else if (key == "size") size = value;
            else if (key == "modify") modified = value;
        }

        if (type == "cdir" || type == "pdir") continue;

        QString entryType{"unknown"};
        if (type == "file") entryType = "file";
        else if (type == "dir") entryType = "directory";
        else if (type.contains("slink")) entryType = "link";

        QVariantHash entry{
            {"name", QString::fromUtf8(line.sliced(space + 1))},
            {"type", entryType}
        };

        if (!size.isEmpty()) {
            bool validSize{};
            const auto value = size.toLongLong(&validSize);
            if (!validSize || value < 0) throw sol::error(m_portName + ": invalid MLSD response");
            entry["size"] = value;
        }
        if (!modified.isEmpty()) entry["modified"] = QString::fromLatin1(modified);

        entries.append(entry);
    }

    return uni_cast<sol::table>(ts, entries);
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
Ftp::CtrlResult Ftp::ctrlResponse() const {
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

    return ctrlParser(rxData);
}

Ftp::CtrlResult Ftp::ctrlParser(const QByteArray &rxData) {
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

Ftp::DataResult Ftp::dataResponse(const QByteArray &command) const {
    QByteArray rxData{};

    // try EPSV first
    if (!m_port->write("EPSV", "utf-8", "crlf")) return {.exception = "write failed"};

    auto result = ctrlResponse();
    const auto host = m_port->config().value("remoteHost").toString();
    quint16 port{};

    if (result.code == StatusCode::EnteringExtendedPassiveMode) {
        const auto begin = result.text.lastIndexOf('(');
        const auto end = result.text.indexOf(')', begin + 1);
        if (begin == -1 || end == -1) return {.exception = "invalid EPSV response"};

        const auto value = result.text.sliced(begin + 1, end - begin - 1);
        if (value.size() < 5) return {.exception = "invalid EPSV response"};

        const auto delimiter = value.at(0);
        if (value.at(1) != delimiter || value.at(2) != delimiter || value.back() != delimiter) return {.exception = "invalid EPSV response"};

        const auto number = value.sliced(3, value.size() - 4).toInt();
        if (number <= 0 || number > 65535) return {.exception = "invalid EPSV response"};
        port = static_cast<quint16>(number);
    }
    // fall back to PASV
    else {
        if (result.code == 0) return {.exception = result.exception};

        if (!m_port->write("PASV", "utf-8", "crlf")) return {.exception = "write failed"};

        result = ctrlResponse();
        if (!result.exception.isEmpty()) return {.exception = result.exception};
        if (result.code != StatusCode::EnteringPassiveMode)
            return {.exception = "unexpected ftp response(" + QString::number(result.code) + ")"};

        const auto begin = result.text.lastIndexOf('(');
        const auto end = result.text.indexOf(')', begin + 1);
        if (begin == -1 || end == -1) return {.exception = "invalid PASV response"};

        const auto values = result.text.sliced(begin + 1, end - begin - 1).split(',');
        if (values.size() != 6) return {.exception = "invalid PASV response"};

        const auto high = values.at(4).trimmed().toUShort();
        const auto low = values.at(5).trimmed().toUShort();
        if (high > 255 || low > 255) return {.exception = "invalid PASV response"};

        port = static_cast<quint16>(high << 8 | low);
        if (port == 0) return {.exception = "invalid PASV response"};
    }

    QTcpSocket socket;
    socket.connectToHost(host, port);
    if (!socket.waitForConnected(m_timeout))
        return {.exception = "data connection failed: " + socket.errorString()};

    if (!m_port->write(command, "utf-8", "crlf")) return {.exception = "write failed"};

    result = ctrlResponse();
    if (!result.exception.isEmpty()) return {.exception = result.exception};
    if (result.code != StatusCode::DataConnectionAlreadyOpen && result.code != StatusCode::FileStatusOkay)
        return {.exception = "unexpected ftp response(" + QString::number(result.code) + ")"};

    const QDeadlineTimer deadline(m_timeout);
    while (socket.state() != QAbstractSocket::UnconnectedState) {
        if (deadline.hasExpired()) {
            socket.abort();
            return {.exception = "data read timeout"};
        }
        socket.waitForReadyRead(10);
        rxData += socket.readAll();
    }
    rxData += socket.readAll();

    result = ctrlResponse();
    if (!result.exception.isEmpty()) return {.exception = result.exception};
    if (result.code != StatusCode::ClosingDataConnection && result.code != StatusCode::RequestedFileActionOkay)
        return {.exception = "unexpected ftp response(" + QString::number(result.code) + ")"};

    return {.data = rxData};
}
