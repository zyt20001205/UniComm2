#include "portModule/tcpServer.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
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

QVariantMap TcpServer::info() {
    if (m_tcpServer == nullptr) return {};
    const bool status = m_tcpServer->isListening();
    const QString localAddress = m_tcpServerLocalAddress;
    const QString localPort = QString::number(m_tcpServerLocalPort);
    QVariantList peerList;
    for (const QTcpSocket *tcpServerPeer: m_tcpServerPeerHash) {
        QMap<QString, QVariant> peerInfo;
        peerInfo["peerAddress"] = tcpServerPeer->peerAddress().toString();
        peerInfo["peerPort"] = tcpServerPeer->peerPort();
        peerList.append(peerInfo);
    }

    QVariantMap infoMap;
    infoMap["status"] = status;
    infoMap["localAddress"] = localAddress;
    infoMap["localPort"] = localPort;
    infoMap["peerList"] = peerList;
    return infoMap;
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
        emit appendLog(QString("%1 started on %2:%3").arg(m_portName, m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort)), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 started on %3:%4").arg(timestamp, m_portName, m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort));
        return true;
    }
    emit appendLog(QString("%1 open failed: %2").arg(m_portName, m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed: %3").arg(timestamp, m_portName, m_tcpServer->errorString());
    return false;
}

void TcpServer::close() {
    if (m_tcpServer == nullptr) return;
    m_tcpServer->close();
    for (QTcpSocket *tcpServerPeer: m_tcpServerPeerHash) {
        if (tcpServerPeer) {
            tcpServerPeer->disconnectFromHost();
            if (tcpServerPeer->state() != QAbstractSocket::UnconnectedState) {
                tcpServerPeer->waitForDisconnected(1000);
            }
            tcpServerPeer->deleteLater();
        }
    }
    m_tcpServerPeerHash.clear();
    emit togglePort(false);
    emit appendLog("tcp server closed", "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "tcp server closed");
}

bool TcpServer::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

bool TcpServer::write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) {
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
    return handleWrite(f_txData, peerIp);
}

QByteArray TcpServer::read(const int timeout, const int length, const QString &rxFormat) {
    QScopedValueRollback rxFormatRollback(m_rxFormat);
    if (!rxFormat.isEmpty()) m_rxFormat = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        for (QTcpSocket *tcpServerPeer: m_tcpServerPeerHash) {
            m_syncMode = true;
            m_bufferSize = 0;
            rxData = handleRead(timeout, length, tcpServerPeer);
            m_syncMode = false;
            if (!rxData.isEmpty()) break;
        }
    }
    return rxData;
}

QByteArray TcpServer::read(const int timeout, const int length, const QString &peerIp, const QString &rxFormat) {
    QScopedValueRollback rxFormatRollback(m_rxFormat);
    if (!rxFormat.isEmpty()) m_rxFormat = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        if (!m_tcpServerPeerHash.contains(peerIp)) {
            emit appendLog(QString("%1 not found").arg(peerIp), "error");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2 not found").arg(timestamp, peerIp);
            return {};
        }
        QTcpSocket *tcpServerPeer = m_tcpServerPeerHash[peerIp];
        m_syncMode = true;
        m_bufferSize = 0;
        rxData = handleRead(timeout, length, tcpServerPeer);
        m_syncMode = false;
    }
    return rxData;
}

// TcpServer private
void TcpServer::handleNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *tcpServerPeer = m_tcpServer->nextPendingConnection();
        handleConnected(tcpServerPeer);
        connect(tcpServerPeer, &QTcpSocket::readyRead, this, [this, tcpServerPeer] { handleReadyRead(tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::disconnected, this, [this, tcpServerPeer] { handleDisconnected(tcpServerPeer); });
        connect(tcpServerPeer, &QTcpSocket::errorOccurred, this, [this, tcpServerPeer] { handleError(tcpServerPeer); });
    }
}

void TcpServer::handleServerError() {
    if (m_tcpServer->serverError() == QAbstractSocket::SocketTimeoutError) return;
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
    emit togglePort(false);
    emit appendLog(QString("%1 error: %2").arg(m_portName, m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portName, m_tcpServer->errorString());
}

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_tcpServerPeerHash.insert(peerIp, tcpServerPeer);
    emit appendLog(QString("%1 accepts connection from %2").arg(m_portName, peerIp), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 accepts connection from %3").arg(timestamp, m_portName, peerIp);
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_tcpServerPeerHash.remove(peerIp);
    tcpServerPeer->deleteLater();
    emit appendLog(QString("%1 lost connection from %2").arg(m_portName, peerIp), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 lost connection from %3").arg(timestamp, m_portName, peerIp);
}

void TcpServer::handleReadyRead(QTcpSocket *tcpServerPeer) {
    QByteArray rxData;
    if (m_syncMode) {
        const auto newBufferSize = tcpServerPeer->bytesAvailable();
        rxData = tcpServerPeer->peek(newBufferSize);
        handleLog("rx", rxData.mid(m_bufferSize), tcpServerPeer);
        m_bufferSize = newBufferSize;
    } else {
        rxData = tcpServerPeer->readAll();
        handleLog("rx", rxData, tcpServerPeer);
    }
    m_rxBuffer = rxData;
}

void TcpServer::handleError(QTcpSocket *tcpServerPeer) {
    if (tcpServerPeer->error() == QAbstractSocket::SocketTimeoutError) return;
    // if (tcpServerPeer->isOpen()) {
    //     tcpServerPeer->close();
    // }
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    // emit togglePort(false);
    emit appendLog(QString("%1 error: %2").arg(peerIp, tcpServerPeer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, peerIp, tcpServerPeer->errorString());
}

bool TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return false;
    }
    if (peerIp.isEmpty()) {
        for (QTcpSocket *tcpServerPeer: m_tcpServerPeerHash) {
            tcpServerPeer->write(f_txData);
            handleLog("tx", f_txData, tcpServerPeer);
        }
    } else {
        if (!m_tcpServerPeerHash.contains(peerIp)) {
            emit appendLog("peer not found", "error");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "peer not found");
            return false;
        }
        QTcpSocket *tcpServerPeer = m_tcpServerPeerHash[peerIp];
        tcpServerPeer->write(f_txData);
        handleLog("tx", f_txData, tcpServerPeer);
    }
    return true;
}

QByteArray TcpServer::handleRead(const int timeout, const int length, QTcpSocket *tcpServerPeer) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (tcpServerPeer->bytesAvailable() == length) {
            QByteArray rxData = tcpServerPeer->readAll();
            return rxData;
        }
        tcpServerPeer->waitForReadyRead(10);
    }
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    emit appendLog(QString("%1 timeout").arg(peerIp), "error");
    return {};
}

void TcpServer::handleLog(const QString &mode, const QByteArray &data, const QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    if (mode == "tx") {
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3] %4").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerIp, txMessage);
        emit appendLog(txMessage, mode);
    } else {
        // rx message reformat
        QString rxMessage;
        // 1: encode rx message according to rx format
        if (m_rxFormat == "hex") rxMessage = data.toHex(' ').toUpper();
        else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(data);
        else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(data);
        // 2: add port info
        rxMessage = QString("[%1:%2 &lt;- %3] %4").arg(m_tcpServerLocalAddress, QString::number(m_tcpServerLocalPort), peerIp, rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
