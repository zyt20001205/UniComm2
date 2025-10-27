#include "portModule/tcpClient.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QTcpSocket>

#include "suffix.h"

// TcpClient public
TcpClient::TcpClient(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()),
      m_tcpClientRemoteAddress(portConfig["tcpClientRemoteAddress"].toString()),
      m_tcpClientRemotePort(portConfig["tcpClientRemotePort"].toInt()),
      m_txFormat(portConfig["txFormat"].toString()),
      m_txSuffix(portConfig["txSuffix"].toString()),
      m_rxFormat(portConfig["rxFormat"].toString()) {
}

void TcpClient::reload(const QJsonObject &portConfig) {
    m_tcpClientRemoteAddress = portConfig["tcpClientRemoteAddress"].toString();
    m_tcpClientRemotePort = portConfig["tcpClientRemotePort"].toInt();
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
}

QVariantMap TcpClient::info() {
    if (m_tcpClient == nullptr) return{};
    QString status;
    switch (m_tcpClient->state()) {
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
    const QString localAddress = m_tcpClientLocalAddress;
    const QString localPort = QString::number(m_tcpClientLocalPort);
    const QString remoteAddress = m_tcpClientRemoteAddress;
    const QString remotePort = QString::number(m_tcpClientRemotePort);

    QVariantMap infoMap;
    infoMap["status"] = status;
    infoMap["localAddress"] = localAddress;
    infoMap["localPort"] = localPort;
    infoMap["remoteAddress"] = remoteAddress;
    infoMap["remotePort"] = remotePort;
    return infoMap;
}

bool TcpClient::open() {
    // port init
    if (m_tcpClient == nullptr) {
        m_tcpClient = new QTcpSocket(this);
        connect(m_tcpClient, &QTcpSocket::connected, this, &TcpClient::handleConnected);
        connect(m_tcpClient, &QTcpSocket::disconnected, this, &TcpClient::handleDisconnected);
        connect(m_tcpClient, &QTcpSocket::readyRead, this, &TcpClient::handleReadyRead);
        connect(m_tcpClient, &QTcpSocket::errorOccurred, this, &TcpClient::handleError);
    }
    m_tcpClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_tcpClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // open port
    m_tcpClient->connectToHost(m_tcpClientRemoteAddress, m_tcpClientRemotePort);
    emit togglePort(true);
    emit appendLog(QString("%1 connecting to %2:%3").arg(m_portName, m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 connecting to %3:%4").arg(timestamp, m_portName, m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
    return true;
}

void TcpClient::close() {
    if (m_tcpClient == nullptr) return;
    switch (m_tcpClient->state()) {
        case QAbstractSocket::ConnectedState:
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::HostLookupState:
            m_tcpClient->disconnectFromHost();
            break;
        default:
            break;
    }
    emit togglePort(false);
}

bool TcpClient::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback txFormatRollback(m_txFormat);
    QScopedValueRollback txSuffixRollback(m_txSuffix);
    if (!txFormat.isEmpty()) m_txFormat = txFormat;
    if (!txSuffix.isEmpty()) m_txSuffix = txSuffix;
    // 1: remove space if tx format is hex
    QByteArray f_txData = txData;
    if (m_txFormat == "hex") f_txData = QByteArray::fromHex(txData);
    // 2: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray TcpClient::read(const int timeout, const int length, const QString &rxFormat) {
    QScopedValueRollback rxFormatRollback(m_rxFormat);
    if (!rxFormat.isEmpty()) m_rxFormat = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
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

// TcpClient private
void TcpClient::handleConnected() {
    m_tcpClientLocalAddress = m_tcpClient->localAddress().toString();
    m_tcpClientLocalPort = m_tcpClient->localPort();
    emit appendLog(QString("%1 connected to %2:%3").arg(m_portName, m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 connected to %3:%4").arg(timestamp, m_portName, m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleDisconnected() {
    emit appendLog(QString("%1 %2:%3").arg("tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleReadyRead() {
    QByteArray rxData;
    if (m_syncMode) {
        const auto newBufferSize = m_tcpClient->bytesAvailable();
        rxData = m_tcpClient->peek(newBufferSize);
        handleLog("rx", rxData.mid(m_bufferSize));
        m_bufferSize = newBufferSize;
    } else {
        rxData = m_tcpClient->readAll();
        handleLog("rx", rxData);
    }
    m_rxBuffer = rxData;
}

void TcpClient::handleError() {
    if (m_tcpClient->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_tcpClient->isOpen()) {
        m_tcpClient->close();
    }
    emit togglePort(false);
    emit appendLog(QString("%1 error: %2").arg(m_portName, m_tcpClient->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portName, m_tcpClient->errorString());
}

bool TcpClient::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_tcpClient == nullptr || !m_tcpClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return false;
    }
    m_tcpClient->write(f_txData);
    handleLog("tx", f_txData);
    return true;
}

QByteArray TcpClient::handleRead(const int timeout, const int length) {
    // check port status
    if (m_tcpClient == nullptr || !m_tcpClient->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (m_tcpClient->bytesAvailable() == length) {
            QByteArray rxData = m_tcpClient->readAll();
            return rxData;
        }
        m_tcpClient->waitForReadyRead(10);
    }
    emit appendLog(QString("%1 timeout").arg(m_portName), "error");
    return {};
}

void TcpClient::handleLog(const QString &mode, const QByteArray &data) {
    if (mode == "tx") {
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                          QString::number(m_tcpClientRemotePort), txMessage);
        emit appendLog(txMessage, mode);
    } else {
        // rx message reformat
        QString rxMessage;
        // 1: encode rx message according to rx format
        if (m_rxFormat == "hex") rxMessage = data.toHex(' ').toUpper();
        else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(data);
        else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(data);
        // 2: add port info
        rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                          QString::number(m_tcpClientRemotePort), rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
