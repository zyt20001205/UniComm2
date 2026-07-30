#include "port/webSocketClient.h"

#include <QEventLoop>
#include <QScopedValueRollback>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "globals.h"
#include "util/uniCast.h"

namespace {
void waitForMessage(QWebSocket *socket) {
    QEventLoop eventLoop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(socket, &QWebSocket::binaryMessageReceived, &eventLoop, &QEventLoop::quit);
    QObject::connect(socket, &QWebSocket::textMessageReceived, &eventLoop, &QEventLoop::quit);
    QObject::connect(socket, &QWebSocket::disconnected, &eventLoop, &QEventLoop::quit);
    timer.start(10);
    eventLoop.exec();
}
}

// public
WebSocketClient::WebSocketClient(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(portConfig["bufferSize"].toInt()) {
}

WebSocketClient::~WebSocketClient() {
    close();
}

int WebSocketClient::type() {
    return PortType::WebSocketClient;
}

QJsonObject WebSocketClient::config() {
    return m_portConfig;
}

QVariantHash WebSocketClient::info() {
    QString status{};
    if (m_webSocketClient == nullptr) {
        status = "unconnected";
    } else {
        switch (m_webSocketClient->state()) {
            case QAbstractSocket::UnconnectedState: status = "unconnected";
                break;
            case QAbstractSocket::HostLookupState: status = "looking up host";
                break;
            case QAbstractSocket::ConnectingState: status = "connecting";
                break;
            case QAbstractSocket::ConnectedState: status = "connected";
                break;
            case QAbstractSocket::ClosingState: status = "closing";
                break;
            case QAbstractSocket::BoundState: status = "bound to local address";
                break;
            default: status = "unknown";
        }
    }
    const QUrl url(m_portConfig["url"].toString());
    return {
        {"status", status},
        {"localHost", m_localHost},
        {"localPort", QString::number(m_localPort)},
        {"remoteHost", url.host()},
        {"remotePort", QString::number(url.port(url.scheme() == "wss" ? 443 : 80))},
        {"bufferSize", QString::number(m_portConfig["bufferSize"].toInt())},
        {"bufferUsed", QString::number(m_buffer.used())}
    };
}

bool WebSocketClient::open() {
    if (m_webSocketClient == nullptr) {
        m_webSocketClient = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
        connect(m_webSocketClient, &QWebSocket::connected, this, &WebSocketClient::handleConnected);
        connect(m_webSocketClient, &QWebSocket::disconnected, this, &WebSocketClient::handleDisconnected);
        connect(m_webSocketClient, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &message) { handleMessage(message); });
        connect(m_webSocketClient, &QWebSocket::textMessageReceived, this, [this](const QString &message) { handleMessage(message.toUtf8()); });
        connect(m_webSocketClient, &QWebSocket::errorOccurred, this, &WebSocketClient::handleError);
    }
    if (m_monitorTimer == nullptr) {
        m_monitorTimer = new QTimer(this);
        m_monitorTimer->setInterval(16);
        m_monitorTimer->setSingleShot(false);
        connect(m_monitorTimer, &QTimer::timeout, this, &WebSocketClient::handleUpdate);
    }
    if (m_webSocketClient->state() != QAbstractSocket::UnconnectedState) return true;

    m_buffer.clear();
    m_buffer.resetStatistics();
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("connecting to %1").arg(m_portConfig["url"].toString()));

    QEventLoop eventLoop;
    QTimer timer;
    bool finished = false;
    bool timedOut = false;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &eventLoop, [&] {
        timedOut = true;
        eventLoop.quit();
    });
    connect(m_webSocketClient, &QWebSocket::connected, &eventLoop, [&] {
        finished = true;
        eventLoop.quit();
    });
    connect(m_webSocketClient, &QWebSocket::errorOccurred, &eventLoop, [&] {
        finished = true;
        eventLoop.quit();
    });

    m_webSocketClient->open(QUrl(m_portConfig["url"].toString()));
    timer.start(30000);
    if (!finished) eventLoop.exec();
    if (m_webSocketClient->state() != QAbstractSocket::ConnectedState) {
        if (timedOut) {
            m_webSocketClient->abort();
            emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "connection timed out");
        }
        return false;
    }

    m_activeTimer.start();
    const QVariantHash session{
        {"active", true},
        {"capacity", m_portConfig["bufferSize"].toInt()},
        {"lifetime", uni_cast<QLifetime>(qint64{}).value}
    };
    emit refreshPort(m_portConfig["portName"].toString(), session);
    return true;
}

void WebSocketClient::close() {
    if (m_webSocketClient == nullptr) return;

    switch (m_webSocketClient->state()) {
        case QAbstractSocket::ConnectedState:
            emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("disconnecting from %1").arg(m_portConfig["url"].toString()));
            m_webSocketClient->close();
            break;
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::HostLookupState:
            emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("connection to %1 cancelled").arg(m_portConfig["url"].toString()));
            m_webSocketClient->abort();
            break;
        default:
            break;
    }
    if (m_monitorTimer) m_monitorTimer->stop();
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
}

void WebSocketClient::clear() {
    m_buffer.clear();
}

void WebSocketClient::monitor(const bool enabled) {
    if (m_monitorTimer == nullptr || m_webSocketClient == nullptr
        || m_webSocketClient->state() != QAbstractSocket::ConnectedState)
        return;
    if (enabled) {
        handleUpdate();
        m_monitorTimer->start();
    } else {
        m_monitorTimer->stop();
    }
}

bool WebSocketClient::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!txFormat.isEmpty()) m_portConfig["txFormat"] = txFormat;
    if (!txSuffix.isEmpty()) m_portConfig["txSuffix"] = txSuffix;
    QByteArray f_txData = txData;
    if (m_portConfig["txSuffix"].toString() == "crlf") f_txData += "\r\n";
    else if (m_portConfig["txSuffix"].toString() == "modbus crc") f_txData += uni_cast<ModbusCRC>(f_txData).value;
    else if (m_portConfig["txSuffix"].toString() == "modbus lrc") f_txData += uni_cast<ModbusLRC>(f_txData).value;
    return handleWrite(f_txData);
}

QByteArray WebSocketClient::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

QByteArray WebSocketClient::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout);
}

// private
void WebSocketClient::handleConnected() {
    m_localHost = m_webSocketClient->localAddress().toString();
    m_localPort = m_webSocketClient->localPort();
    emit connected();
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("connected to %1").arg(m_portConfig["url"].toString()));
}

void WebSocketClient::handleDisconnected() {
    if (m_monitorTimer) m_monitorTimer->stop();
    clear();
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit disconnected();
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("disconnected from %1").arg(m_portConfig["url"].toString()));
}

void WebSocketClient::handleMessage(const QByteArray &message) {
    if (m_buffer.write(message) != message.size()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "buffer overflow");
        close();
        return;
    }
    emit readyRead();
    handleLog(LogLevel::Receive, message);
}

void WebSocketClient::handleError() {
    if (m_webSocketClient->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_monitorTimer) m_monitorTimer->stop();
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), m_webSocketClient->errorString());
}

bool WebSocketClient::handleWrite(const QByteArray &f_txData) {
    if (m_webSocketClient == nullptr || !m_webSocketClient->isValid()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return false;
    }
    if (m_portConfig["messageType"].toString() == "text")
        m_webSocketClient->sendTextMessage(QString::fromUtf8(f_txData));
    else
        m_webSocketClient->sendBinaryMessage(f_txData);
    handleLog(LogLevel::Transmit, f_txData);
    return true;
}

QByteArray WebSocketClient::handleRead(const int length, const int timeout) {
    if (m_webSocketClient == nullptr || !m_webSocketClient->isValid()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.used() < length) {
        if (deadline.hasExpired()) break;
        waitForMessage(m_webSocketClient);
    }
    return m_buffer.read(length);
}

QByteArray WebSocketClient::handleReadUntil(const QByteArray &text, const int timeout) {
    if (m_webSocketClient == nullptr || !m_webSocketClient->isValid()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    if (text.isEmpty()) return {};
    const QDeadlineTimer deadline(timeout);
    QByteArray data = m_buffer.readUntil(text);
    while (data.isEmpty()) {
        if (deadline.hasExpired()) break;
        waitForMessage(m_webSocketClient);
        data = m_buffer.readUntil(text);
    }
    return data;
}

void WebSocketClient::handleUpdate() {
    const auto statistics = m_buffer.statistics();
    const QVariantHash session{
        {"used", statistics.used},
        {"lifetime", uni_cast<QLifetime>(m_activeTimer.elapsed()).value},
        {"readCount", statistics.readCount},
        {"readBytes", statistics.readBytes},
        {"writeCount", statistics.writeCount},
        {"writeBytes", statistics.writeBytes}
    };
    emit refreshPort(m_portConfig["portName"].toString(), session);
}

void WebSocketClient::handleLog(const int type, const QByteArray &data) {
    QString message{};
    const QString format = type == LogLevel::Transmit ? m_portConfig["txFormat"].toString() : m_portConfig["rxFormat"].toString();
    if (format == "raw") {
        message.reserve(data.size() * 4);
        for (const char c: data) message += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
    } else if (format == "hex") message = data.toHex(' ').toUpper();
    else if (format == "ascii") message = QString::fromLatin1(data);
    else message = QString::fromUtf8(data);

    if (type == LogLevel::Transmit) {
        emit appendLog(type, QString("[%1:%2 -&gt; %3]").arg(m_localHost, QString::number(m_localPort), m_portConfig["url"].toString()), message);
    } else {
        emit appendLog(type, QString("[%1:%2 &lt;- %3]").arg(m_localHost, QString::number(m_localPort), m_portConfig["url"].toString()), message);
    }
}
