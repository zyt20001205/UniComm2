#include "portModule/udpSocket.h"

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

QHash<QString, QVariant> UdpSocket::info() {
    bool status;
    if (m_udpSocket->state() == QAbstractSocket::ConnectedState)
        status = true;
    else
        status = false;
    const QString localAddress = m_udpSocketLocalAddress;
    const QString localPort = QString::number(m_udpSocketLocalPort);
    const QString remoteAddress = m_udpSocketRemoteAddress;
    const QString remotePort = QString::number(m_udpSocketRemotePort);

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["localAddress"] = localAddress;
    infoHash["localPort"] = localPort;
    infoHash["remoteAddress"] = remoteAddress;
    infoHash["remotePort"] = remotePort;
    return infoHash;
}

bool UdpSocket::open() {
    // port init
    if (m_udpSocket == nullptr) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, [this] { handleRead(0, 0); });
        connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &UdpSocket::handleError);
    }
    // open port
    if (!m_udpSocket->bind(QHostAddress(m_udpSocketLocalAddress), m_udpSocketLocalPort)) {
        emit appendLog(QString("udp socket open failed: %1").arg(m_udpSocket->errorString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] udp socket open failed: %2").arg(timestamp, m_udpSocket->errorString());
        return false;
    }
    m_udpSocket->connectToHost(m_udpSocketRemoteAddress, m_udpSocketRemotePort);
    emit appendLog(QString("udp socket opened: %1:%2->%3:%4").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                                  QString::number(m_udpSocketRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] udp socket opened: %2:%3->%4:%5").arg(timestamp, m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                                    QString::number(m_udpSocketRemotePort));
    return true;
}

void UdpSocket::close() {
    if (m_udpSocket == nullptr) return;
    m_udpSocket->close();
}

void UdpSocket::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    writeData(txData);
}

void UdpSocket::writeData(const QByteArray &txData) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog("udp socket is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] udp socket is not opened").arg(timestamp);
        return;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData);
}

QString UdpSocket::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray UdpSocket::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// UdpSocket private
void UdpSocket::handleError() {
}

void UdpSocket::handleWrite(const QByteArray &f_txData) {
    m_udpSocket->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                      QString::number(m_udpSocketRemotePort), txMessage);
    emit appendLog(txMessage, "tx");
}

QByteArray UdpSocket::handleRead(const int timeout, const int length) {
    QByteArray rxData = m_udpSocket->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!m_udpSocket->waitForReadyRead(10)) {
                rxData += m_udpSocket->readAll();
            }
            time += 10;
            if (time >= timeout) {
                rxData = "timeout";
                break;
            }
        }
    }
    m_rxBuffer = rxData;
    // rx message reformat
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_udpSocketLocalAddress, QString::number(m_udpSocketLocalPort), m_udpSocketRemoteAddress,
                                                      QString::number(m_udpSocketRemotePort), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}
