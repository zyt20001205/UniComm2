#include "portModule/udpSocket.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QUdpSocket>

#include "suffix.h"

// UdpSocket public
UdpSocket::UdpSocket(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()),
      m_udpSocketLocalAddress(portConfig["udpSocketLocalAddress"].toString()),
      m_udpSocketLocalPort(portConfig["udpSocketLocalPort"].toInt()),
      m_udpSocketRemoteAddress(portConfig["udpSocketRemoteAddress"].toString()),
      m_udpSocketRemotePort(portConfig["udpSocketRemotePort"].toInt()),
      m_txFormat(portConfig["txFormat"].toString()),
      m_txSuffix(portConfig["txSuffix"].toString()),
      m_rxFormat(portConfig["rxFormat"].toString()) {
}

void UdpSocket::reload(const QJsonObject &portConfig) {
    m_portName = portConfig["portName"].toString();
    m_udpSocketLocalAddress = portConfig["udpSocketLocalAddress"].toString();
    m_udpSocketLocalPort = portConfig["udpSocketLocalPort"].toInt();
    m_udpSocketRemoteAddress = portConfig["udpSocketRemoteAddress"].toString();
    m_udpSocketRemotePort = portConfig["udpSocketRemotePort"].toInt();
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
}

QVariantMap UdpSocket::info() {
    if (m_udpSocket == nullptr) return {};
    bool status;
    if (m_udpSocket->state() == QAbstractSocket::ConnectedState)
        status = true;
    else
        status = false;
    const QString localAddress = m_udpSocketLocalAddress;
    const QString localPort = QString::number(m_udpSocketLocalPort);
    const QString remoteAddress = m_udpSocketRemoteAddress;
    const QString remotePort = QString::number(m_udpSocketRemotePort);

    QVariantMap infoMap;
    infoMap["status"] = status;
    infoMap["localAddress"] = localAddress;
    infoMap["localPort"] = localPort;
    infoMap["remoteAddress"] = remoteAddress;
    infoMap["remotePort"] = remotePort;
    return infoMap;
}

bool UdpSocket::open() {
    // port init
    if (m_udpSocket == nullptr) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpSocket::handleReadyRead);
        connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &UdpSocket::handleError);
    }
    // open port
    if (!m_udpSocket->bind(QHostAddress(m_udpSocketLocalAddress), m_udpSocketLocalPort)) {
        emit appendLog(QString("%1 open failed: %2").arg(m_portName, m_udpSocket->errorString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 open failed: %3").arg(timestamp, m_portName, m_udpSocket->errorString());
        return false;
    }
    m_udpSocket->connectToHost(m_udpSocketRemoteAddress, m_udpSocketRemotePort);
    emit togglePort(true);
    emit appendLog(QString("%1 opened: %2:%3->%4:%5").arg(m_portName, m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                          QString::number(m_udpSocketRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened: %3:%4->%5:%6").arg(timestamp, m_portName, m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                            QString::number(m_udpSocketRemotePort));
    return true;
}

void UdpSocket::close() {
    if (m_udpSocket == nullptr) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    emit togglePort(false);
}

bool UdpSocket::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

QByteArray UdpSocket::read(const int timeout, const int length, const QString &rxFormat) {
    QScopedValueRollback rxFormatRollback(m_rxFormat);
    if (!rxFormat.isEmpty()) m_rxFormat = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        // TODO: UDP LATENCY IS SO LOW, SYNC MODE CAN'T BE SET IN TIME
        m_syncMode = true;
        m_bufferSize = 0;
        rxData = handleRead(timeout, length);
        m_syncMode = false;
    }
    return rxData;
}

// UdpSocket private
void UdpSocket::handleReadyRead() {
    QByteArray rxData;
    if (m_syncMode) {
        const auto newBufferSize = m_udpSocket->bytesAvailable();
        rxData = m_udpSocket->peek(newBufferSize);
        handleLog("rx", rxData.mid(m_bufferSize));
        m_bufferSize = newBufferSize;
    } else {
        rxData = m_udpSocket->readAll();
        handleLog("rx", rxData);
    }
    m_rxBuffer = rxData;
}

void UdpSocket::handleError() {
    if (m_udpSocket->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    emit togglePort(false);
    emit appendLog(QString("%1 error: %2").arg(m_portName, m_udpSocket->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portName, m_udpSocket->errorString());
}

bool UdpSocket::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return false;
    }
    m_udpSocket->write(f_txData);
    handleLog("tx", f_txData);
    return true;
}

QByteArray UdpSocket::handleRead(const int timeout, const int length) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (m_udpSocket->bytesAvailable() == length) {
            QByteArray rxData = m_udpSocket->readAll();
            return rxData;
        }
        m_udpSocket->waitForReadyRead(10);
    }
    emit appendLog(QString("%1 timeout").arg(m_portName), "error");
    return {};
}

void UdpSocket::handleLog(const QString &mode, const QByteArray &data) {
    if (mode == "tx") {
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                          QString::number(m_udpSocketRemotePort), txMessage);
        emit appendLog(txMessage, mode);
    } else {
        // rx message reformat
        QString rxMessage;
        // 1: encode rx message according to rx format
        if (m_rxFormat == "hex") rxMessage = data.toHex(' ').toUpper();
        else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(data);
        else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(data);
        // 2: add port info
        rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                          QString::number(m_udpSocketRemotePort), rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
