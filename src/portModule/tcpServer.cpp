#include "portModule/tcpServer.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QTcpServer>
#include <QTcpSocket>

#include "globals.h"
#include "utils/suffixUtils.h"


// TODO: RingBuffer rewrite required!!
// TcpServer public
TcpServer::TcpServer(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

TcpServer::~TcpServer() {
    close();
}

int TcpServer::type() {
    return TCPSERVER;
}

QJsonObject TcpServer::config() {
    return m_portConfig;
}

std::unordered_map<std::string, std::string> TcpServer::info() {
    const std::string status = m_tcpServer && m_tcpServer->isListening() ? "opened" : "closed";
    const std::string localHost = m_portConfig["localHost"].toString().toStdString();
    const std::string localPort = QString::number(m_portConfig["localPort"].toInt()).toStdString();
    // QVariantList peerList;
    // for (const QTcpSocket *tcpServerPeer: m_tcpServerPeerHash) {
    //     QMap<QString, QVariant> peerInfo;
    //     peerInfo["peerAddress"] = tcpServerPeer->peerAddress().toString();
    //     peerInfo["peerPort"] = tcpServerPeer->peerPort();
    //     peerList.append(peerInfo);
    // }

    std::unordered_map<std::string, std::string> infoHash{};
    infoHash["status"] = status;
    infoHash["localHost"] = localHost;
    infoHash["localPort"] = localPort;
    // infoMap["peerList"] = peerList;
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
    if (m_tcpServer->listen(QHostAddress(m_portConfig["localHost"].toString()), m_portConfig["localPort"].toInt())) {
        emit refreshPort(m_portConfig["portName"].toString(), true);
        emit appendLog(QString("%1 started on %2:%3").arg(m_portConfig["portName"].toString(), m_portConfig["localHost"].toString(),
                                                          QString::number(m_portConfig["localPort"].toInt())), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 started on %3:%4").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["localHost"].toString(),
                                                            QString::number(m_portConfig["localPort"].toInt()));
        return true;
    }
    emit appendLog(QString("%1 open failed: %2").arg(m_portConfig["portName"].toString(), m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed: %3").arg(timestamp, m_portConfig["portName"].toString(), m_tcpServer->errorString());
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
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

void TcpServer::clear() {
    m_bufferSize = 0;
    m_rxBuffer = {};
}

bool TcpServer::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

bool TcpServer::write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) {
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
    return handleWrite(f_txData, peerIp);
}

QByteArray TcpServer::read(const int length, const int timeout, const QString &rxFormat) {
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

QByteArray TcpServer::read(const int length, const int timeout, const QString &peerIp, const QString &rxFormat) {
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
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_tcpServer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_tcpServer->errorString());
}

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_tcpServerPeerHash.insert(peerIp, tcpServerPeer);
    emit appendLog(QString("%1 accepts connection from %2").arg(m_portConfig["portName"].toString(), peerIp), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 accepts connection from %3").arg(timestamp, m_portConfig["portName"].toString(), peerIp);
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_tcpServerPeerHash.remove(peerIp);
    tcpServerPeer->deleteLater();
    emit appendLog(QString("%1 lost connection from %2").arg(m_portConfig["portName"].toString(), peerIp), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 lost connection from %3").arg(timestamp, m_portConfig["portName"].toString(), peerIp);
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
    // emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(peerIp, tcpServerPeer->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, peerIp, tcpServerPeer->errorString());
}

bool TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
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

QByteArray TcpServer::handleRead(const int length, const int timeout, QTcpSocket *tcpServerPeer) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (tcpServerPeer->bytesAvailable() >= length) {
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
        if (m_portConfig["txFormat"].toString() == "raw") {
            txMessage.reserve(data.size() * 4);
            for (const char c: data) {
                txMessage += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
            }
        } else if (m_portConfig["txFormat"].toString() == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_portConfig["txFormat"].toString() == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_portConfig["txFormat"].toString() == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1:%2 -&gt; %3] %4").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp, txMessage);
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
        rxMessage = QString("[%1:%2 &lt;- %3] %4").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp, rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
