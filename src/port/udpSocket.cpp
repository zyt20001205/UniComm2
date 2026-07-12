#include "port/udpSocket.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QTimer>
#include <QUdpSocket>

#include "globals.h"
#include "util/suffixUtils.h"

// public
UdpSocket::UdpSocket(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(portConfig["bufferSize"].toInt()) {
}

UdpSocket::~UdpSocket() {
    close();
}

int UdpSocket::type() {
    return PortType::UdpSocket;
}

QJsonObject UdpSocket::config() {
    return m_portConfig;
}

QVariantHash UdpSocket::info() {
    const QString status = m_udpSocket && m_udpSocket->state() == QAbstractSocket::ConnectedState ? "connected" : "disconnected";
    const auto localHost = m_portConfig["localHost"].toString();
    const auto localPort = QString::number(m_portConfig["localPort"].toInt());
    const auto remoteHost = m_portConfig["remoteHost"].toString();
    const auto remotePort = QString::number(m_portConfig["remotePort"].toInt());
    const auto bufferSize = QString::number(m_portConfig["bufferSize"].toInt());
    const auto bufferUsed = QString::number(m_buffer.used());

    const QVariantHash infoHash = {
        {"status", status},
        {"localHost", localHost},
        {"localPort", localPort},
        {"remoteHost", remoteHost},
        {"remotePort", remotePort},
        {"bufferSize", bufferSize},
        {"bufferUsed", bufferUsed}
    };
    return infoHash;
}

bool UdpSocket::open() {
    // status check
    if (m_udpSocket == nullptr) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpSocket::handleReadyRead);
        connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &UdpSocket::handleError);
    }
    if (m_monitorTimer == nullptr) {
        m_monitorTimer = new QTimer(this);
        m_monitorTimer->setInterval(16);
        m_monitorTimer->setSingleShot(false);
        connect(m_monitorTimer, &QTimer::timeout, this, &UdpSocket::handleUpdate);
    }
    if (m_udpSocket->state() != QAbstractSocket::UnconnectedState) return true;
    // open port
    if (!m_udpSocket->bind(QHostAddress(m_portConfig["localHost"].toString()), m_portConfig["localPort"].toInt())) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("open failed: %1").arg(m_udpSocket->errorString()));
        return false;
    }
    m_udpSocket->connectToHost(m_portConfig["remoteHost"].toString(), m_portConfig["remotePort"].toInt());
    m_buffer.clear();
    m_buffer.resetStatistics();
    m_activeTimer.start();
    const QVariantHash session{
        {"active", true},
        {"capacity", m_portConfig["bufferSize"].toInt()},
        {"lifetime", lifetimeFormat(0)}
    };
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Info,
                   QString("[%1]").arg(m_portConfig["portName"].toString()),
                   QString("opened: %2:%3->%4:%5")
                   .arg(m_portConfig["portName"].toString(),
                        m_portConfig["localHost"].toString(),
                        QString::number(m_portConfig["localPort"].toInt()),
                        m_portConfig["remoteHost"].toString(),
                        QString::number(m_portConfig["remotePort"].toInt())));
    return true;
}

void UdpSocket::close() {
    // status check
    if (m_udpSocket == nullptr) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    if (m_monitorTimer) m_monitorTimer->stop();
    // port close
    clear();
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Info, QString("[%1]").arg(m_portConfig["portName"].toString()), "closed");
}

void UdpSocket::clear() {
    m_buffer.clear();
}

void UdpSocket::monitor(const bool enabled) {
    if (m_monitorTimer == nullptr || m_udpSocket == nullptr || !m_udpSocket->isOpen()) return;
    if (enabled) {
        handleUpdate();
        m_monitorTimer->start();
    } else {
        m_monitorTimer->stop();
    }
}

bool UdpSocket::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

QByteArray UdpSocket::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

QByteArray UdpSocket::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout);
}

// private
void UdpSocket::handleReadyRead() {
    const auto rxData = m_udpSocket->readAll();
    if (m_buffer.write(rxData) != rxData.size()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "buffer overflow");
        close();
    }
    handleLog(LogLevel::Receive, rxData);
}

void UdpSocket::handleError() {
    if (m_udpSocket->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    if (m_monitorTimer) m_monitorTimer->stop();
    const QVariantHash session{{"active", false}};
    emit refreshPort(m_portConfig["portName"].toString(), session);
    emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("%1").arg(m_udpSocket->errorString()));
}

bool UdpSocket::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return false;
    }
    m_udpSocket->write(f_txData);
    handleLog(LogLevel::Transmit, f_txData);
    return true;
}

QByteArray UdpSocket::handleRead(const int length, const int timeout) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.used() < length) {
        if (deadline.hasExpired()) break;
        m_udpSocket->waitForReadyRead(10);
    }
    return m_buffer.read(length);
}

QByteArray UdpSocket::handleReadUntil(const QByteArray &text, const int timeout) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(LogLevel::Error, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    if (text.isEmpty()) return {};
    const QDeadlineTimer deadline(timeout);
    QByteArray data = m_buffer.readUntil(text);
    while (data.isEmpty()) {
        if (deadline.hasExpired()) break;
        m_udpSocket->waitForReadyRead(10);
        data = m_buffer.readUntil(text);
    }
    return data;
}

void UdpSocket::handleUpdate() {
    const auto statistics = m_buffer.statistics();
    const auto &session = QVariantHash{
        {"used", statistics.used},
        {"lifetime", lifetimeFormat(m_activeTimer.elapsed())},
        {"readCount", statistics.readCount},
        {"readBytes", statistics.readBytes},
        {"writeCount", statistics.writeCount},
        {"writeBytes", statistics.writeBytes}
    };
    emit refreshPort(m_portConfig["portName"].toString(), session);
}

void UdpSocket::handleLog(const int type, const QByteArray &data) {
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
        emit appendLog(type,
                       QString("[%1:%2 -&gt; %3:%4]").
                       arg(m_portConfig["localHost"].toString(),
                           QString::number(m_portConfig["localPort"].toInt()),
                           m_portConfig["remoteHost"].toString(),
                           QString::number(m_portConfig["remotePort"].toInt())),
                       txMessage);
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
        emit appendLog(type,
                       QString("[%1:%2 &lt;- %3:%4] ").
                       arg(m_portConfig["localHost"].toString(),
                           QString::number(m_portConfig["localPort"].toInt()),
                           m_portConfig["remoteHost"].toString(),
                           QString::number(m_portConfig["remotePort"].toInt())),
                       rxMessage);
    }
}
