#include "portModule/sslClient.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QSslSocket>

#include "globals.h"
#include "utils/suffixUtils.h"

// SslClient public
SslClient::SslClient(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
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

std::unordered_map<std::string, std::string> SslClient::info() {
    std::string status;
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
    const std::string localHost = m_sslClientLocalHost.toStdString();
    const std::string localPort = QString::number(m_sslClientLocalPort).toStdString();
    const std::string remoteHost = m_portConfig["remoteHost"].toString().toStdString();
    const std::string remotePort = QString::number(m_portConfig["remotePort"].toInt()).toStdString();

    std::unordered_map<std::string, std::string> infoHash{};
    infoHash["status"] = status;
    infoHash["localHost"] = localHost;
    infoHash["localPort"] = localPort;
    infoHash["remoteHost"] = remoteHost;
    infoHash["remotePort"] = remotePort;
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
                                                         QString::number(m_portConfig["remotePort"].toInt())), "info");
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
    emit refreshPort(m_portConfig["portName"].toString(), false);
}

bool SslClient::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!txFormat.isEmpty()) m_portConfig["txFormat"] = txFormat;
    if (!txSuffix.isEmpty()) m_portConfig["txSuffix"] = txSuffix;
    // 1: remove space if tx format is hex
    QByteArray f_txData = txData;
    if (m_portConfig["txFormat"].toString() == "hex") f_txData = QByteArray::fromHex(txData);
    // 2: append suffix according to tx suffix
    if (m_portConfig["txSuffix"].toString() == "crlf") f_txData += "\r\n";
    else if (m_portConfig["txSuffix"].toString() == "modbus crc") f_txData += modbusCRC(f_txData);
    else if (m_portConfig["txSuffix"].toString() == "modbus lrc") f_txData += modbusLRC(f_txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray SslClient::read(const int timeout, const int length, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
        m_rxBuffer = {};
    }
    // sync mode
    else {
        m_syncMode = true;
        m_bufferSize = 0;
        rxData = handleRead(timeout, length);
        m_syncMode = false;
    }
    return rxData;
}

// SslClient private
void SslClient::handleConnected() {
    m_sslClientLocalHost = m_sslClient->localAddress().toString();
    m_sslClientLocalPort = m_sslClient->localPort();
    emit appendLog(QString("%1 connected to %2:%3").arg(m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                        QString::number(m_portConfig["remotePort"].toInt())), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 connected to %3:%4").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()));
}

void SslClient::handleDisconnected() {
    emit appendLog(QString("%1 %2:%3").arg("tcp client disconnected from", m_portConfig["remoteHost"].toString(), QString::number(m_portConfig["remotePort"].toInt())), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client disconnected from", m_portConfig["remoteHost"].toString(), QString::number(m_portConfig["remotePort"].toInt()));
}

void SslClient::handleReadyRead() {
    QByteArray rxData;
    if (m_syncMode) {
        const auto newBufferSize = m_sslClient->bytesAvailable();
        rxData = m_sslClient->peek(newBufferSize);
        handleLog("rx", rxData.mid(m_bufferSize));
        m_bufferSize = newBufferSize;
    } else {
        rxData = m_sslClient->readAll();
        handleLog("rx", rxData);
    }
    m_rxBuffer = rxData;
}

void SslClient::handleError() {
    if (m_sslClient->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_sslClient->isOpen()) {
        m_sslClient->close();
    }
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_sslClient->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_sslClient->errorString());
}

bool SslClient::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_sslClient == nullptr || !m_sslClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    m_sslClient->write(f_txData);
    handleLog("tx", f_txData);
    return true;
}

QByteArray SslClient::handleRead(const int timeout, const int length) {
    // check port status
    if (m_sslClient == nullptr || !m_sslClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (m_sslClient->bytesAvailable() >= length) {
            QByteArray rxData = m_sslClient->readAll();
            return rxData;
        }
        m_sslClient->waitForReadyRead(10);
    }
    emit appendLog(QString("%1 timeout").arg(m_portConfig["portName"].toString()), "error");
    return {};
}

void SslClient::handleLog(const QString &mode, const QByteArray &data) {
    if (mode == "tx") {
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
        emit appendLog(txMessage, mode);
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
        emit appendLog(rxMessage, mode);
    }
}
