#include "port/tcpServer.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QTcpServer>
#include <QTcpSocket>

#include "globals.h"
#include "port/module/ringBuffer.h"
#include "util/suffixUtils.h"

TcpServer::TcpServer(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

TcpServer::~TcpServer() {
    close();
}

int TcpServer::type() {
    return PortType::TcpServer;
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
    // status check
    if (m_tcpServer == nullptr) {
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);
        connect(m_tcpServer, &QTcpServer::acceptError, this, &TcpServer::handleServerError);
    }
    if (m_tcpServer->isListening()) return true;
    // open port
    // m_tcpServer->setMaxPendingConnections();
    if (m_tcpServer->listen(QHostAddress(m_portConfig["localHost"].toString()), m_portConfig["localPort"].toInt())) {
        const QVariantHash session{{"active", true}};
        emit refreshPort(m_portConfig["portName"].toString(), session);
        emit appendLog(LogLevel::Info,
                       QString("[%1]").arg(m_portConfig["portName"].toString()),
                       QString("started on %1:%2").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt())));
        return true;
    }
    emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("open failed: %1").arg(m_tcpServer->errorString()));
    return false;
}

void TcpServer::close() {
    // status check
    if (m_tcpServer == nullptr) return;
    // port close
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
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), "closed");
}

void TcpServer::clear() {
    for (RingBuffer *buffer: m_bufferHash) {
        buffer->clear();
    }
}

void TcpServer::monitor(const bool enabled) {
    Q_UNUSED(enabled);
    // TODO: Aggregate statistics across active and disconnected peer buffers.
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

QByteArray TcpServer::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    if (m_peerHash.isEmpty()) return {};
    return handleReadUntil(text, timeout, m_peerHash.keys().first());
}

QByteArray TcpServer::readUntil(const QByteArray &text, const int timeout, const QString &peerIp, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout, peerIp);
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
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("%1").arg(m_tcpServer->errorString()));
}

void TcpServer::handleConnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_peerHash.insert(peerIp, tcpServerPeer);
    m_bufferHash.insert(peerIp, new RingBuffer(m_portConfig["bufferSize"].toInt()));
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("accepts connection from %1").arg(peerIp));
}

void TcpServer::handleDisconnected(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    m_peerHash.remove(peerIp);
    tcpServerPeer->deleteLater();
    delete m_bufferHash[peerIp];
    m_bufferHash.remove(peerIp);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("lost connection from %1").arg(peerIp));
}

void TcpServer::handleReadyRead(QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    const auto rxData = tcpServerPeer->readAll();
    auto *buffer = m_bufferHash.value(peerIp, nullptr);
    if (buffer == nullptr) return;
    if (buffer->write(rxData) != rxData.size()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(peerIp), "buffer overflow");
        close();
    }
    handleLog(LogLevel::Receive, rxData, tcpServerPeer);
}

void TcpServer::handleError(const QTcpSocket *tcpServerPeer) {
    if (tcpServerPeer->error() == QAbstractSocket::SocketTimeoutError) return;
    // if (tcpServerPeer->isOpen()) {
    //     tcpServerPeer->close();
    // }
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    emit appendLog(LogLevel::Error, QString("[%1]").arg(peerIp), QString("error: %1").arg(tcpServerPeer->errorString()));
}

bool TcpServer::handleWrite(const QByteArray &f_txData, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return false;
    }
    if (peerIp.isEmpty()) {
        for (QTcpSocket *tcpServerPeer: m_peerHash) {
            tcpServerPeer->write(f_txData);
            handleLog(LogLevel::Transmit, f_txData, tcpServerPeer);
        }
    } else {
        const auto peer = m_peerHash.constFind(peerIp);
        if (peer == m_peerHash.constEnd()) {
            emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "peer not found");
            return false;
        }
        QTcpSocket *tcpServerPeer = peer.value();
        tcpServerPeer->write(f_txData);
        handleLog(LogLevel::Transmit, f_txData, tcpServerPeer);
    }
    return true;
}

QByteArray TcpServer::handleRead(const int length, const int timeout, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    auto *buffer = m_bufferHash.value(peerIp, nullptr);
    auto *peer = m_peerHash.value(peerIp, nullptr);
    if (buffer == nullptr || peer == nullptr) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "peer not found");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (buffer->used() < length) {
        if (deadline.hasExpired()) break;
        peer->waitForReadyRead(10);
        buffer = m_bufferHash.value(peerIp, nullptr);
        peer = m_peerHash.value(peerIp, nullptr);
        if (buffer == nullptr || peer == nullptr) return {};
    }
    return buffer->read(length);
}

QByteArray TcpServer::handleReadUntil(const QByteArray &text, const int timeout, const QString &peerIp) {
    // check port status
    if (m_tcpServer == nullptr || !m_tcpServer->isListening()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    if (text.isEmpty()) return {};
    auto *buffer = m_bufferHash.value(peerIp, nullptr);
    auto *peer = m_peerHash.value(peerIp, nullptr);
    if (buffer == nullptr || peer == nullptr) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "peer not found");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    QByteArray data = buffer->readUntil(text);
    while (data.isEmpty()) {
        if (deadline.hasExpired()) break;
        peer->waitForReadyRead(10);
        buffer = m_bufferHash.value(peerIp, nullptr);
        peer = m_peerHash.value(peerIp, nullptr);
        if (buffer == nullptr || peer == nullptr) return {};
        data = buffer->readUntil(text);
    }
    return data;
}

void TcpServer::handleLog(const int type, const QByteArray &data, const QTcpSocket *tcpServerPeer) {
    const QString peerIp = tcpServerPeer->peerAddress().toString() + ":" + QString::number(tcpServerPeer->peerPort());
    if (type == LogLevel::Transmit) {
        // tx message reformat
        QString txMessage{};
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
        emit appendLog(type, QString("[%1:%2 -&gt; %3]").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp), txMessage);
    } else {
        // rx message reformat
        QString rxMessage{};
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
        emit appendLog(type, QString("[%1:%2 &lt;- %3]").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()), peerIp), rxMessage);
    }
}
