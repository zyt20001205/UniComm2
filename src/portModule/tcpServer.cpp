#include "portModule/tcpServer.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QTcpServer>
#include <QTcpSocket>

#include "globals.h"
#include "utils/ringBuffer.h"
#include "utils/suffixUtils.h"

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

QVariantHash TcpServer::info() {
    const QString status = m_tcpServer && m_tcpServer->isListening() ? "opened" : "closed";
    const auto localHost = m_portConfig["localHost"].toString();
    const auto localPort = QString::number(m_portConfig["localPort"].toInt());
    QVariantHash infoHash = {
        {"status", status},
        {"localHost", localHost},
        {"localPort", localPort}
    };
    const int capacity = m_portConfig["bufferSize"].toInt();
    int index = 1;
    QVariantHash peerInfoHash{};
    for (const auto &peerIp: m_peerHash.keys()) {
        const auto *peer = m_peerHash.value(peerIp);
        peerInfoHash["peerAddress"] = peer->peerAddress().toString();
        peerInfoHash["peerPort"] = peer->peerPort();
        auto *buffer = m_bufferHash.value(peerIp);
        peerInfoHash["used"] = QString::number(buffer->used());
        peerInfoHash["capacity"] = QString::number(capacity);
        infoHash.insert("peer" + QString::number(index++), peerInfoHash);
    }
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
                                                          QString::number(m_portConfig["localPort"].toInt())), LOG_INFO);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 started on %3:%4").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["localHost"].toString(),
                                                            QString::number(m_portConfig["localPort"].toInt()));
        return true;
    }
    emit appendLog(QString("%1 open failed: %2").arg(m_portConfig["portName"].toString(), m_tcpServer->errorString()), LOG_ERROR);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed: %3").arg(timestamp, m_portConfig["portName"].toString(), m_tcpServer->errorString());
    return false;
}

void TcpServer::close() {
    if (m_tcpServer == nullptr) return;
    m_tcpServer->close();
    for (QTcpSocket *tcpServerPeer: m_peerHash) {
        if (tcpServerPeer) {
            tcpServerPeer->disconnectFromHost();
            if (tcpServerPeer->state() != QAbstractSocket::UnconnectedState) {
                tcpServerPeer->waitForDisconnected(1000);
            }
            tcpServerPeer->deleteLater();
        }
    }
    m_peerHash.clear();
    qDeleteAll(m_bufferHash);
    m_bufferHash.clear();
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

void TcpServer::clear() {
    for (RingBuffer *buffer: m_bufferHash) {
        buffer->clear();
    }
}

bool TcpServer::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

bool TcpServer::write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!txFormat.isEmpty()) m_portConfig["txFormat"] = txFormat;
    if (!txSuffix.isEmpty()) m_portConfig["txSuffix"] = txSuffix;
    QByteArray f_txData = txData;
    if (m_portConfig["txSuffix"].toString() == "crlf") f_txData += "\r\n";
    else if (m_portConfig["txSuffix"].toString() == "modbus crc") f_txData += modbusCRC(f_txData);
    else if (m_portConfig["txSuffix"].toString() == "modbus lrc") f_txData += modbusLRC(f_txData);
    // call handle write
    return handleWrite(f_txData, peerIp);
}

QByteArray TcpServer::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    if (m_peerHash.isEmpty()) return {};
    return handleRead(length, timeout, m_peerHash.keys().first());
}

QByteArray TcpServer::read(const int length, const int timeout, const QString &peerIp, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout, peerIp);
}

// private
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
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_tcpServer->errorString()), LOG_ERROR);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_tcpServer->errorString());
}

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_peerHash.insert(peerIp, tcpServerPeer);
    m_bufferHash.insert(peerIp, new RingBuffer(m_portConfig["bufferSize"].toInt()));
    emit appendLog(QString("%1 accepts connection from %2").arg(m_portConfig["portName"].toString(), peerIp), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 accepts connection from %3").arg(timestamp, m_portConfig["portName"].toString(), peerIp);
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_peerHash.remove(peerIp);
    tcpServerPeer->deleteLater();
    delete m_bufferHash[peerIp];
    m_bufferHash.remove(peerIp);
    emit appendLog(QString("%1 lost connection from %2").arg(m_portConfig["portName"].toString(), peerIp), LOG_INFO);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 lost connection from %3").arg(timestamp, m_portConfig["portName"].toString(), peerIp);
}

void TcpServer::handleReadyRead(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    const auto rxData = tcpServerPeer->readAll();
    m_bufferHash[peerIp]->write(rxData);
    handleLog(LOG_RX, rxData, tcpServerPeer);
}

void TcpServer::handleError(QTcpSocket *tcpServerPeer) {
    if (tcpServerPeer->error() == QAbstractSocket::SocketTimeoutError) return;
    // if (tcpServerPeer->isOpen()) {
    //     tcpServerPeer->close();
    // }
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    // emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(peerIp, tcpServerPeer->errorString()), LOG_ERROR);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, peerIp, tcpServerPeer->errorString());
}

bool TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    if (peerIp.isEmpty()) {
        for (QTcpSocket *tcpServerPeer: m_peerHash) {
            tcpServerPeer->write(f_txData);
            handleLog(LOG_TX, f_txData, tcpServerPeer);
        }
    } else {
        if (!m_peerHash.contains(peerIp)) {
            emit appendLog("peer not found", LOG_ERROR);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "peer not found");
            return false;
        }
        QTcpSocket *tcpServerPeer = m_peerHash[peerIp];
        tcpServerPeer->write(f_txData);
        handleLog(LOG_TX, f_txData, tcpServerPeer);
    }
    return true;
}

QByteArray TcpServer::handleRead(const int length, const int timeout, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_bufferHash[peerIp]->used() < length) {
        if (deadline.hasExpired()) break;
        m_peerHash[peerIp]->waitForReadyRead(10);
    }
    return m_bufferHash[peerIp]->read(length);
}

void TcpServer::handleLog(const int type, const QByteArray &data, const QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
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
        txMessage = QString("[%1:%2 -&gt; %3] %4").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp, txMessage);
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
        rxMessage = QString("[%1:%2 &lt;- %3] %4").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp, rxMessage);
        emit appendLog(rxMessage, type);
    }
}
