#include "api/ftp.h"

#include <QTimer>
#include <sol/error.hpp>

#include "globals.h"
#include "port/tcpServer.h"
#include "port/portModule.h"

// public
Ftp::Ftp(QObject *parent)
    : QObject(parent) {
}

void Ftp::init(const std::string &portName, const int timeout) {
    const auto name = QString::fromStdString(portName);
    const auto port = g_port->m_portHash.constFind(name);
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");
    if (port.value()->type() != PortType::TcpServer) throw sol::error(portName + " is not a tcp server");

    m_portName = portName;
    m_timeout = timeout;
    m_port = static_cast<TcpServer *>(port.value());
}

void Ftp::start(const sol::table &options) {
    const auto username = QByteArray::fromStdString(options.get_or("username", std::string{}));
    const auto password = QByteArray::fromStdString(options.get_or("password", std::string{}));
    const bool allowAnonymous = options.get_or("allowAnonymous", true);
    const int maxAttempts = options.get_or("maxAttempts", 1);

    accept();
    stateSet(StatusCode::ServiceReady);

    // login
    QByteArray _username{};
    int attempts{};

    while (true) {
        QString exception{};
        int statusCode{};

        QMetaObject::invokeMethod(m_port, [&, this] {
            auto parsed = parser(m_port->readUntil("\r\n", m_timeout, m_peer, "utf-8"));
            exception = parsed.take("exception").toString();
            if (!exception.isEmpty()) return;

            statusCode = parsed.value("statusCode").toInt();
            switch (m_state) {
                case StatusCode::ServiceReady: {
                    switch (statusCode) {
                        case StatusCode::UserNameOkay:
                            _username = parsed.value("username").toByteArray();
                            break;
                        case StatusCode::SystemType:
                        case StatusCode::SyntaxError:
                            break;
                        default:
                            statusCode = StatusCode::BadSequenceOfCommands;
                            break;
                    }
                }
                break;
                case StatusCode::UserLoggedIn: {
                    switch (statusCode) {
                        case StatusCode::SystemType:
                        case StatusCode::PathnameCreated:
                        case StatusCode::SyntaxError:
                            break;
                        default:
                            statusCode = StatusCode::BadSequenceOfCommands;
                            break;
                    }
                    break;
                }
                case StatusCode::UserNameOkay: {
                    switch (statusCode) {
                        case StatusCode::UserLoggedIn: {
                            const auto _password = parsed.value("password").toByteArray();
                            const bool authenticated =
                                    // anonymous
                                    (allowAnonymous && (_username.compare("anonymous", Qt::CaseInsensitive) == 0 || _username.compare("ftp", Qt::CaseInsensitive) == 0))
                                    // username & password
                                    || (!username.isEmpty() && _username == username && _password == password);
                            statusCode = authenticated ? StatusCode::UserLoggedIn : StatusCode::NotLoggedIn;
                            break;
                        }
                        case StatusCode::SystemType:
                        case StatusCode::SyntaxError:
                            break;
                        default:
                            statusCode = StatusCode::BadSequenceOfCommands;
                            break;
                    }
                }
                break;
                default: statusCode = StatusCode::BadSequenceOfCommands;
                    break;
            }

            if (!m_port->write(assembler(statusCode), m_peer, "utf-8", "null")) {
                exception = "write failed";
            }
        }, Qt::BlockingQueuedConnection);

        if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());

        stateSet(statusCode);
        if (statusCode == StatusCode::NotLoggedIn && ++attempts >= maxAttempts) return;
    }
}

// private
void Ftp::accept() {
    m_peer.clear();

    QEventLoop loop{};
    connect(m_port, &TcpServer::connected, &loop, [this, &loop](const QString &peerIp) {
        m_peer = peerIp;
        loop.quit();
    }, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));

    QString exception{};

    QMetaObject::invokeMethod(m_port, [&exception, this] {
        if (!m_port->open()) exception = "open failed";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());

    if (m_timeout >= 0) QTimer::singleShot(m_timeout, &loop, &QEventLoop::quit);
    loop.exec();
    if (m_peer.isEmpty()) throw sol::error(m_portName + ": accept timeout");

    QMetaObject::invokeMethod(m_port, [&exception, this] {
        const auto response = assembler(StatusCode::ServiceReady);
        if (!m_port->write(response, m_peer, "utf-8", "null")) {
            exception = "write failed";
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());
}

void Ftp::stateSet(const int state) {
    switch (state) {
        case StatusCode::ServiceReady:
        case StatusCode::UserLoggedIn:
        case StatusCode::UserNameOkay:
            m_state = state;
            break;
        case StatusCode::NotLoggedIn:
            m_state = StatusCode::ServiceReady;
            break;
        default:
            break;
    }
}

QVariantHash Ftp::parser(const QByteArray &rxData) {
    if (rxData.isEmpty()) return {{"exception", "read timeout"}};

    auto line = rxData;
    if (line.endsWith("\r\n")) line.chop(2);

    const auto space = line.indexOf(' ');
    const auto command = (space == -1 ? line : line.first(space)).toUpper();

    qsizetype argumentStart = space;
    while (argumentStart != -1 && argumentStart < line.size() && line.at(argumentStart) == ' ') {
        ++argumentStart;
    }
    const auto argument = argumentStart == -1 ? QByteArray{} : line.mid(argumentStart);

    QVariantHash parsed{{"statusCode", StatusCode::SyntaxError}};
    if (command == "USER") {
        parsed["statusCode"] = StatusCode::UserNameOkay;
        parsed["username"] = argument;
    } else if (command == "PASS") {
        parsed["statusCode"] = StatusCode::UserLoggedIn;
        parsed["password"] = argument;
    } else if (command == "PWD") {
        parsed["statusCode"] = StatusCode::PathnameCreated;
    } else if (command == "SYST") {
        parsed["statusCode"] = StatusCode::SystemType;
    }
    return parsed;
}

QByteArray Ftp::assembler(const int statusCode) {
    QByteArray message{};
    switch (statusCode) {
        case StatusCode::SystemType:
            message = "UNIX Type: L8";
            break;
        case StatusCode::ServiceReady:
            message = "UniComm FTP Service ready";
            break;
        case StatusCode::UserLoggedIn:
            message = "Login successful";
            break;
        case StatusCode::PathnameCreated:
            message = "\"" + g_workspaceUrl.toLocalFile().toUtf8() + "\" is current directory";
            break;
        case StatusCode::UserNameOkay:
            message = "Password required";
            break;
        case StatusCode::SyntaxError:
            message = "Syntax error, command unrecognized";
            break;
        case StatusCode::BadSequenceOfCommands:
            message = "Bad sequence of commands";
            break;
        case StatusCode::NotLoggedIn:
            message = "Login incorrect";
            break;
        default:
            break;
    }

    auto response = QByteArray::number(statusCode);
    if (!message.isEmpty()) {
        response += ' ';
        response += message;
    }
    return response + "\r\n";
}
