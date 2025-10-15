#include "portModule/tcpServer.h"

#include <QTcpServer>
#include <QTcpSocket>

#include "suffix.h"

// TcpServer public
TcpServer::TcpServer(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()),
      m_tcpServerLocalAddress(portConfig["tcpServerLocalAddress"].toString()),
      m_tcpServerLocalPort(portConfig["tcpServerLocalPort"].toInt()),
      m_txFormat(portConfig["txFormat"].toString()),
      m_txSuffix(portConfig["txSuffix"].toString()),
      m_rxFormat(portConfig["rxFormat"].toString()) {
}

void TcpServer::reload(const QJsonObject &portConfig) {
    m_tcpServerLocalAddress = portConfig["tcpServerLocalAddress"].toString();
    m_tcpServerLocalPort = portConfig["tcpServerLocalPort"].toInt();
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> TcpServer::info() {
    const bool status = m_tcpServer->isListening();
    const QString localAddress = m_tcpServerLocalAddress;
    const QString localPort = QString::number(m_tcpServerLocalPort);
    QList<QVariant> peerList;
    for (const QTcpSocket *tcpServerPeer: m_tcpServerPeerList) {
        QMap<QString, QVariant> peerInfo;
        peerInfo["peerAddress"] = tcpServerPeer->peerAddress().toString();
        peerInfo["peerPort"] = tcpServerPeer->peerPort();
        peerList.append(peerInfo);
    }

    QHash<QString, QVariant> infoHash;
    infoHash["status"] = status;
    infoHash["localAddress"] = localAddress;
    infoHash["localPort"] = localPort;
    infoHash["peerList"] = peerList;
    return infoHash;
}

bool TcpServer::open() {
    // port init
    if (m_tcpServer == nullptr) {
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);
        connect(m_tcpServer, &QTcpServer::acceptError, this, &TcpServer::handleServerError);
    }
    // m_tcpServer->setMaxPendingConnections();
    // open port
    if (m_tcpServer->listen(QHostAddress(m_tcpServerLocalAddress), m_tcpServerLocalPort)) {
        emit togglePort(true);
        emit appendLog(QString("%1 %2:%3").arg("tcp server started on", m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort)), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "tcp server started on", m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort));
        return true;
    }
    emit appendLog(QString("%1: %2").arg("tcp server open failed", m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2: %3").arg(timestamp, "tcp server open failed", m_tcpServer->errorString());
    return false;
}

void TcpServer::close() {
    if (m_tcpServer == nullptr) return;
    m_tcpServer->close();
    for (QTcpSocket *tcpServerPeer: m_tcpServerPeerList) {
        if (tcpServerPeer) {
            tcpServerPeer->disconnectFromHost();
            if (tcpServerPeer->state() != QAbstractSocket::UnconnectedState) {
                tcpServerPeer->waitForDisconnected(1000);
            }
            tcpServerPeer->deleteLater();
        }
    }
    m_tcpServerPeerList.clear();
    emit togglePort(false);
    emit appendLog("tcp server closed", "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "tcp server closed");
}

bool TcpServer::writeText(const QString &txText) {
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

bool TcpServer::writeText(const QString &txText, const QString &peerIp) {
    // tx text reformat
    QString f_txText = txText;
    // 1: remove space if tx format is hex
    if (m_txFormat == "hex") f_txText.remove(" ");
    // 2: convert to byte array
    QByteArray txData;
    if (m_txFormat == "hex") txData = QByteArray::fromHex(f_txText.toUtf8());
    else if (m_txFormat == "ascii") txData = f_txText.toLatin1();
    else /* txFormat == "utf-8" */ txData = f_txText.toUtf8();
    return writeData(txData, peerIp);
}

bool TcpServer::writeData(const QByteArray &txData) {
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

bool TcpServer::writeData(const QByteArray &txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return false;
    }
    // tx data reformat
    QByteArray f_txData = txData;
    // 1: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc8 maxim") f_txData += crc8Maxim(txData);
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(txData);
    else; /* m_txSuffix == "null" */
    // call handle write
    handleWrite(f_txData, peerIp);
    return true;
}

QString TcpServer::readText(const int timeout, const int length, const QString &peerIp) {
    const QByteArray rxData = readData(timeout, length, peerIp);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray TcpServer::readData(const int timeout, const int length, const QString &peerIp) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        foreach(QTcpSocket* tcpServerPeer, m_tcpServerPeerList) {
            disconnect(tcpServerPeer, &QTcpSocket::readyRead, this, nullptr);
            rxData = handleRead(timeout, length, tcpServerPeer);
            connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer] { handleRead(0, 0, tcpServerPeer); });
        }
    }
    return rxData;
}

// TcpServer private
void TcpServer::handleNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *tcpServerPeer = m_tcpServer->nextPendingConnection();
        handleConnected(tcpServerPeer);
        connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer] { handleRead(0, 0, tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::disconnected, this, [this, tcpServerPeer] { handleDisconnected(tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::errorOccurred, this, [this, tcpServerPeer](QAbstractSocket::SocketError error) { handleError(tcpServerPeer); });
    }
}

void TcpServer::handleServerError() {
};

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    m_tcpServerPeerList.append(tcpServerPeer);
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    emit appendLog(QString("%1 %2:%3").arg("new client connected", peerAddress, peerPort), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "new client connected", peerAddress, peerPort);
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    m_tcpServerPeerList.removeOne(tcpServerPeer);
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    emit appendLog(QString("%1 %2:%3").arg("client disconnected", peerAddress, peerPort), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3:%4").arg(timestamp, "client disconnected", peerAddress, peerPort);
}

void TcpServer::handleError(QTcpSocket *tcpServerPeer) {
}

bool TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return false;
    }
    if (peerIp == "") {
        foreach(QTcpSocket* tcpServerPeer, m_tcpServerPeerList) {
            tcpServerPeer->write(f_txData);
        }
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3] %4").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), "broadcast", txMessage);
        emit appendLog(txMessage, "tx");
    } else {
        QTcpSocket *tcpServerPeer = nullptr;
        foreach(QTcpSocket* peer, m_tcpServerPeerList) {
            if (peerIp == QString("%1:%2").arg(peer->peerAddress().toString(), QString::number(peer->peerPort()))) {
                tcpServerPeer = peer;
                break;
            }
        }
        if (tcpServerPeer == nullptr) {
            emit appendLog("peer not found", "error");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "peer not found");
            return false;
        }
        tcpServerPeer->write(f_txData);
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
        // 2: add port info
        QString peerAddress = tcpServerPeer->peerAddress().toString();
        QString peerPort = QString::number(tcpServerPeer->peerPort());
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress, peerPort, txMessage);
        emit appendLog(txMessage, "tx");
    }
    return true;
}

QByteArray TcpServer::handleRead(const int timeout, const int length, QTcpSocket *tcpServerPeer) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog("tcp server is not opened", "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "tcp server is not opened");
        return {};
    }
    QByteArray rxData = tcpServerPeer->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (!tcpServerPeer->waitForReadyRead(10)) {
                rxData += tcpServerPeer->readAll();
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
    QString peerAddress = tcpServerPeer->peerAddress().toString();
    QString peerPort = QString::number(tcpServerPeer->peerPort());
    QString rxMessage;
    // 1: encode rx message according to rx format
    if (m_rxFormat == "hex") rxMessage = rxData.toHex(' ').toUpper();
    else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(rxData);
    else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(rxData);
    // 2: add port info
    rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerAddress,
                                                      peerPort, rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}
