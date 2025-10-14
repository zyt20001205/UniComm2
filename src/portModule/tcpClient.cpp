#include "portModule/tcpClient.h"

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

QHash<QString, QVariant> TcpClient::info() {
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
    const QString localAddress = m_tcpClient->localAddress().toString();
    const QString localPort = QString::number(m_tcpClient->localPort());
    const QString remoteAddress = m_tcpClientRemoteAddress;
    const QString remotePort = QString::number(m_tcpClientRemotePort);

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["localAddress"] = localAddress;
    infoHash["localPort"] = localPort;
    infoHash["remoteAddress"] = remoteAddress;
    infoHash["remotePort"] = remotePort;
    return infoHash;
}

bool TcpClient::open() {
    // port init
    if (m_tcpClient == nullptr) {
        m_tcpClient = new QTcpSocket(this);
        connect(m_tcpClient, &QTcpSocket::connected, this, &TcpClient::handleConnected);
        connect(m_tcpClient, &QTcpSocket::disconnected, this, &TcpClient::handleDisconnected);
        connect(m_tcpClient, &QTcpSocket::readyRead, this, [this] { handleRead(0, 0); });
        connect(m_tcpClient, &QTcpSocket::errorOccurred, this, &TcpClient::handleError);
    }
    m_tcpClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_tcpClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // open port
    m_tcpClient->connectToHost(m_tcpClientRemoteAddress, m_tcpClientRemotePort);
    return true;
}

void TcpClient::close() {
    if (m_tcpClient == nullptr) return;
    m_tcpClient->disconnectFromHost();
}

bool TcpClient::writeText(const QString &txText) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    return writeData(txData);
}

bool TcpClient::writeData(const QByteArray &txData) {
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    return handleWrite(f_txData);
}

QString TcpClient::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray TcpClient::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_tcpClient, &QTcpSocket::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_tcpClient, &QTcpSocket::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// TcpClient private
void TcpClient::handleConnected() {
    m_tcpClientLocalAddress = m_tcpClient->localAddress().toString();
    m_tcpClientLocalPort = m_tcpClient->localPort();
    emit appendLog(QString("%1 %2:%3").arg("tcp client connected to", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client connected to", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleDisconnected() {
    emit appendLog(QString("%1 %2:%3").arg("tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort)), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp client disconnected from", m_tcpClientRemoteAddress, QString::number(m_tcpClientRemotePort));
}

void TcpClient::handleError() {
}

bool TcpClient::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_tcpClient == nullptr || !m_tcpClient->isOpen()) {
        emit appendLog("tcp client is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] tcp client is not opened").arg(timestamp);
        return false;
    }
    m_tcpClient->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                      QString::number(m_tcpClientRemotePort), txMessage);
    emit appendLog(txMessage, "tx");
    return true;
}

QByteArray TcpClient::handleRead(const int timeout, const int length) {
    // check port status
    if (m_tcpClient == nullptr || !m_tcpClient->isOpen()) {
        emit appendLog("tcp client is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] tcp client is not opened").arg(timestamp);
        return {};
    }
    QByteArray rxData = m_tcpClient->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!m_tcpClient->waitForReadyRead(10)) {
                rxData += m_tcpClient->readAll();
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
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpClientLocalAddress, QString::number(m_tcpClientLocalPort), m_tcpClientRemoteAddress,
                                                      QString::number(m_tcpClientRemotePort), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}
