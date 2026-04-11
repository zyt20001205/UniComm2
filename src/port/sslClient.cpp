#include "port/sslClient.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QSslSocket>

#include "globals.h"
#include "utils/suffixUtils.h"

// public
SslClient::SslClient(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(portConfig["bufferSize"].toInt()) {
}

SslClient::~SslClient() {
    close();
}

int SslClient::type() {
    return SSLCLIENT;
}

QJsonObject SslClient::config() {
    return m_portConfig;
}

QVariantHash SslClient::info() {
    QString status{};
    if (m_sslClient == nullptr) {
        status = "unconnected";
    } else {
        switch (m_sslClient->state()) {
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
    const auto localHost = m_sslClientLocalHost;
    const auto localPort = QString::number(m_sslClientLocalPort);
    const auto remoteHost = m_portConfig["remoteHost"].toString();
    const auto remotePort = QString::number(m_portConfig["remotePort"].toInt());
    const auto bufferSize = QString::number(m_portConfig["bufferSize"].toInt());
    const auto bufferUsed = QString::number(m_buffer.used());

    const QVariantHash infoHash = {
        {"status", status},
        {"localHost", localHost},
        {"localPort", localPort},
        {"remoteHost", remoteHost},
        {"remotePort", remotePort},
        {"bufferSize", bufferSize},
        {"bufferUsed", bufferUsed}
    };
    return infoHash;
}

bool SslClient::open() {
    // port init
    if (m_sslClient == nullptr) {
        m_sslClient = new QSslSocket(this);
        // TODO: This is not safe!!! test only
        m_sslClient->setPeerVerifyMode(QSslSocket::VerifyNone);
        connect(m_sslClient, &QSslSocket::connected, this, &SslClient::handleConnected);
        connect(m_sslClient, &QSslSocket::disconnected, this, &SslClient::handleDisconnected);
        connect(m_sslClient, &QSslSocket::readyRead, this, &SslClient::handleReadyRead);
        connect(m_sslClient, &QSslSocket::errorOccurred, this, &SslClient::handleError);
    }
    m_sslClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_sslClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // open port
    m_sslClient->connectToHostEncrypted(m_portConfig["remoteHost"].toString(), m_portConfig["remotePort"].toInt());
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 connecting to %2:%3").arg(m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                         QString::number(m_portConfig["remotePort"].toInt())), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 connecting to %3:%4").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                           QString::number(m_portConfig["remotePort"].toInt()));
    return true;
}

void SslClient::close() {
    if (m_sslClient == nullptr) return;
    switch (m_sslClient->state()) {
        case QAbstractSocket::ConnectedState: {
            m_sslClient->disconnectFromHost();
        }
        break;
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::HostLookupState: {
            m_sslClient->abort();
        }
        break;
        default:
            break;
    }
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

void SslClient::clear() {
    m_buffer.clear();
}

bool SslClient::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!txFormat.isEmpty()) m_portConfig["txFormat"] = txFormat;
    if (!txSuffix.isEmpty()) m_portConfig["txSuffix"] = txSuffix;
    QByteArray f_txData = txData;
    if (m_portConfig["txSuffix"].toString() == "crlf") f_txData += "\r\n";
    else if (m_portConfig["txSuffix"].toString() == "modbus crc") f_txData += modbusCRC(f_txData);
    else if (m_portConfig["txSuffix"].toString() == "modbus lrc") f_txData += modbusLRC(f_txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray SslClient::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

QByteArray SslClient::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout);
}

// private
void SslClient::handleConnected() {
    m_sslClientLocalHost = m_sslClient->localAddress().toString();
    m_sslClientLocalPort = m_sslClient->localPort();
    emit appendLog(QString("%1 connected to %2:%3").arg(m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                        QString::number(m_portConfig["remotePort"].toInt())), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 connected to %3:%4").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()));
}

void SslClient::handleDisconnected() {
    emit appendLog(QString("%1 %2:%3").arg("tcp client disconnected from", m_portConfig["remoteHost"].toString(), QString::number(m_portConfig["remotePort"].toInt())), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client disconnected from", m_portConfig["remoteHost"].toString(), QString::number(m_portConfig["remotePort"].toInt()));
}

void SslClient::handleReadyRead() {
    const auto rxData = m_sslClient->readAll();
    m_buffer.write(rxData);
    handleLog(LOG_RX, rxData);
}

void SslClient::handleError() {
    if (m_sslClient->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_sslClient->isOpen()) {
        m_sslClient->close();
    }
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_sslClient->errorString()), LOG_ERROR);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_sslClient->errorString());
}

bool SslClient::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_sslClient == nullptr || !m_sslClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    m_sslClient->write(f_txData);
    handleLog(LOG_TX, f_txData);
    return true;
}

QByteArray SslClient::handleRead(const int length, const int timeout) {
    // check port status
    if (m_sslClient == nullptr || !m_sslClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.used() < length) {
        if (deadline.hasExpired()) break;
        m_sslClient->waitForReadyRead(10);
    }
    return m_buffer.read(length);
}

QByteArray SslClient::handleReadUntil(const QByteArray &text, const int timeout) {
    // check port status
    if (m_sslClient == nullptr || !m_sslClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.distance(text) == -1) {
        if (deadline.hasExpired()) break;
        m_sslClient->waitForReadyRead(10);
    }
    return m_buffer.read(m_buffer.distance(text));
}

void SslClient::handleLog(const int type, const QByteArray &data) {
    if (type == LOG_TX) {
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_portConfig["txFormat"].toString() == "raw") {
            txMessage.reserve(data.size() * 4);
            for (const char c: data) {
                txMessage += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
            }
        } else if (m_portConfig["txFormat"].toString() == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_portConfig["txFormat"].toString() == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_portConfig["txFormat"].toString() == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_sslClientLocalHost, QString::number(m_sslClientLocalPort), m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()), txMessage);
        emit appendLog(txMessage, type);
    } else {
        // rx message reformat
        QString rxMessage;
        // 1: encode rx message according to rx format
        if (m_portConfig["rxFormat"].toString() == "raw") {
            rxMessage.reserve(data.size() * 4);
            for (const char c: data) {
                rxMessage += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
            }
        } else if (m_portConfig["rxFormat"].toString() == "hex") rxMessage = data.toHex(' ').toUpper();
        else if (m_portConfig["rxFormat"].toString() == "ascii") rxMessage = QString::fromLatin1(data);
        else /* m_portConfig["rxFormat"].toString() == "utf-8" */ rxMessage = QString::fromUtf8(data);
        // 2: add port info
        rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_sslClientLocalHost, QString::number(m_sslClientLocalPort), m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()), rxMessage);
        emit appendLog(rxMessage, type);
    }
}
