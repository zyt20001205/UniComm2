#include "api/ftp.h"

#include <QThread>
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
    m_options.username = QByteArray::fromStdString(options.get_or("username", std::string{}));
    m_options.password = QByteArray::fromStdString(options.get_or("password", std::string{}));
    m_options.allowAnonymous = options.get_or("allowAnonymous", true);
    m_options.maxAttempts = options.get_or("maxAttempts", 1);

    connect(m_port, &TcpServer::connected, this, &Ftp::handleConnected);
    connect(m_port, &TcpServer::readyRead, this, &Ftp::handleReadyRead);
    connect(m_port, &TcpServer::disconnected, this, &Ftp::handleDisconnected);

    QString exception{};
    QMetaObject::invokeMethod(m_port, [&exception, this] {
        if (!m_port->open()) exception = "open failed";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(m_portName + ": " + exception.toStdString());

    QEventLoop loop{};
    QTimer timer{};
    connect(&timer, &QTimer::timeout, &loop, [&loop] {
        if (QThread::currentThread()->isInterruptionRequested()) loop.quit();
    });
    timer.start(100);
    loop.exec();

    disconnect(m_port, nullptr, this, nullptr);
    m_sessions.clear();
}

// private
void Ftp::handleConnected(const QString &peerIp) {
    m_sessions.insert(peerIp, Session{});

    bool written{};
    QMetaObject::invokeMethod(m_port, [&written, this, &peerIp] {
        written = m_port->write(assembler(StatusCode::ServiceReady), peerIp, "utf-8", "null");
    }, Qt::BlockingQueuedConnection);
    if (!written) m_sessions.remove(peerIp);
}

void Ftp::handleDisconnected(const QString &peerIp) {
    m_sessions.remove(peerIp);
}

void Ftp::handleReadyRead(const QString &peerIp) {
    auto it = m_sessions.find(peerIp);
    if (it == m_sessions.end()) return;
    auto &session = it.value();

    while (true) {
        QByteArray rxData{};
        QMetaObject::invokeMethod(m_port, [&rxData, this, &peerIp] {
            rxData = m_port->readUntil("\r\n", 0, peerIp, "utf-8");
        }, Qt::BlockingQueuedConnection);
        if (rxData.isEmpty()) return;

        const auto parsed = parser(session, rxData);
        const int statusCode = parsed.value("statusCode").toInt();
        const auto exception = parsed.value("exception").toString();

        bool written{};
        QMetaObject::invokeMethod(m_port, [&written, &peerIp, &statusCode, this] {
            written = m_port->write(assembler(statusCode), peerIp, "utf-8", "null");
        }, Qt::BlockingQueuedConnection);
        if (!written) return;

        if (!exception.isEmpty()) {
            QMetaObject::invokeMethod(m_port, [peerIp, this] {
                m_port->disconnectPeer(peerIp);
            }, Qt::BlockingQueuedConnection);
            return;
        }
    }
}

QVariantHash Ftp::parser(Session &session, const QByteArray &rxData) const {
    auto line = rxData;
    if (line.endsWith("\r\n")) line.chop(2);

    const auto space = line.indexOf(' ');
    const auto command = (space == -1 ? line : line.first(space)).toUpper();

    qsizetype argumentStart = space;
    while (argumentStart != -1 && argumentStart < line.size() && line.at(argumentStart) == ' ') {
        ++argumentStart;
    }

    QVariantHash parsed{};
    int statusCode = StatusCode::SyntaxError;
    const auto argument = argumentStart == -1 ? QByteArray{} : line.mid(argumentStart);

    if (command == "USER") statusCode = StatusCode::UserNameOkay;
    else if (command == "PASS") statusCode = StatusCode::UserLoggedIn;
    else if (command == "PWD") statusCode = StatusCode::PathnameCreated;
    else if (command == "SYST") statusCode = StatusCode::SystemType;

    switch (session.state) {
        case StatusCode::ServiceReady:
            switch (statusCode) {
                case StatusCode::SystemType:
                case StatusCode::SyntaxError:
                    break;
                case StatusCode::UserNameOkay:
                    session.username = argument;
                    session.state = StatusCode::UserNameOkay;
                    break;
                default:
                    statusCode = StatusCode::BadSequenceOfCommands;
                    break;
            }
            break;
        case StatusCode::UserLoggedIn:
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
        case StatusCode::UserNameOkay:
            switch (statusCode) {
                case StatusCode::SystemType:
                case StatusCode::SyntaxError:
                    break;
                case StatusCode::UserLoggedIn: {
                    const auto password = argument;
                    const bool authenticated =
                            // anonymous
                            (m_options.allowAnonymous
                             && (session.username.compare("anonymous", Qt::CaseInsensitive) == 0
                                 || session.username.compare("ftp", Qt::CaseInsensitive) == 0))
                            // username & password
                            || (!m_options.username.isEmpty()
                                && session.username == m_options.username
                                && password == m_options.password);
                    if (authenticated) {
                        session.state = StatusCode::UserLoggedIn;
                    } else {
                        statusCode = StatusCode::NotLoggedIn;
                        session.state = StatusCode::ServiceReady;
                        session.username.clear();
                        if (++session.attempts >= m_options.maxAttempts) {
                            parsed["exception"] = "maximum login attempts exceeded";
                        }
                    }
                    break;
                }
                default:
                    statusCode = StatusCode::BadSequenceOfCommands;
                    break;
            }
            break;
        default:
            statusCode = StatusCode::BadSequenceOfCommands;
            break;
    }

    parsed["statusCode"] = statusCode;
    return parsed;
}

QByteArray Ftp::assembler(const int statusCode) {
    QByteArray message{};
    switch (statusCode) {
        case StatusCode::SystemType: message = "UNIX Type: L8";
            break;
        case StatusCode::ServiceReady: message = "UniComm FTP Service ready";
            break;
        case StatusCode::UserLoggedIn: message = "Login successful";
            break;
        case StatusCode::PathnameCreated: message = "\"" + g_workspaceUrl.toLocalFile().toUtf8() + "\" is current directory";
            break;
        case StatusCode::UserNameOkay: message = "Password required";
            break;
        case StatusCode::SyntaxError: message = "Syntax error, command unrecognized";
            break;
        case StatusCode::BadSequenceOfCommands: message = "Bad sequence of commands";
            break;
        case StatusCode::NotLoggedIn: message = "Login incorrect";
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
