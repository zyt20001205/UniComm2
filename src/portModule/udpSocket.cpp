#include "portModule/udpSocket.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QUdpSocket>

#include "globals.h"
#include "utils/suffixUtils.h"

// UdpSocket public
UdpSocket::UdpSocket(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(1024) {
}

UdpSocket::~UdpSocket() {
    close();
}

int UdpSocket::type() {
    return UDPSOCKET;
}

QJsonObject UdpSocket::config() {
    return m_portConfig;
}

std::unordered_map<std::string, std::string> UdpSocket::info() {
    const std::string status = m_udpSocket && m_udpSocket->state() == QAbstractSocket::ConnectedState ? "connected" : "disconnected";
    const std::string localHost = m_portConfig["localHost"].toString().toStdString();
    const std::string localPort = QString::number(m_portConfig["localPort"].toInt()).toStdString();
    const std::string remoteHost = m_portConfig["remoteHost"].toString().toStdString();
    const std::string remotePort = QString::number(m_portConfig["remotePort"].toInt()).toStdString();

    std::unordered_map<std::string, std::string> infoHash{};
    infoHash["status"] = status;
    infoHash["localHost"] = localHost;
    infoHash["localPort"] = localPort;
    infoHash["remoteHost"] = remoteHost;
    infoHash["remotePort"] = remotePort;
    return infoHash;
}

bool UdpSocket::open() {
    // port init
    if (m_udpSocket == nullptr) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpSocket::handleReadyRead);
        connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &UdpSocket::handleError);
    }
    // open port
    if (!m_udpSocket->bind(QHostAddress(m_portConfig["localHost"].toString()), m_portConfig["localPort"].toInt())) {
        emit appendLog(QString("%1 open failed: %2").arg(m_portConfig["portName"].toString(), m_udpSocket->errorString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 open failed: %3").arg(timestamp, m_portConfig["portName"].toString(), m_udpSocket->errorString());
        return false;
    }
    m_udpSocket->connectToHost(m_portConfig["remoteHost"].toString(), m_portConfig["remotePort"].toInt());
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened: %2:%3->%4:%5").arg(m_portConfig["portName"].toString(), m_portConfig["localHost"].toString(),
                                                          QString::number(m_portConfig["localPort"].toInt()), m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt())), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened: %3:%4->%5:%6").arg(timestamp, m_portConfig["portName"].toString(), m_portConfig["localHost"].toString(),
                                                            QString::number(m_portConfig["localPort"].toInt()), m_portConfig["remoteHost"].toString(),
                                                            QString::number(m_portConfig["remotePort"].toInt()));
    return true;
}

void UdpSocket::close() {
    if (m_udpSocket == nullptr) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

void UdpSocket::clear() {
    m_buffer.clear();
}

bool UdpSocket::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

QByteArray UdpSocket::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

// UdpSocket private
void UdpSocket::handleReadyRead() {
    const auto rxData = m_udpSocket->readAll();
    m_buffer.write(rxData);
    handleLog("rx", rxData);
}

void UdpSocket::handleError() {
    if (m_udpSocket->error() == QAbstractSocket::SocketTimeoutError) return;
    if (m_udpSocket->isOpen()) {
        m_udpSocket->close();
    }
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_udpSocket->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_udpSocket->errorString());
}

bool UdpSocket::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    m_udpSocket->write(f_txData);
    handleLog("tx", f_txData);
    return true;
}

QByteArray UdpSocket::handleRead(const int length, const int timeout) {
    // check port status
    if (m_udpSocket == nullptr || !m_udpSocket->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.used() < length) {
        if (deadline.hasExpired()) break;
        m_udpSocket->waitForReadyRead(10);
    }
    return m_buffer.read(length);
}

void UdpSocket::handleLog(const QString &mode, const QByteArray &data) {
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
        txMessage = QString("[%1:%2 -&gt; %3:%4] %5").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()),
                                                          m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()), txMessage);
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
        rxMessage = QString("[%1:%2 &lt;- %3:%4] %5").arg(m_portConfig["localHost"].toString(), QString::number(m_portConfig["localPort"].toInt()),
                                                          m_portConfig["remoteHost"].toString(),
                                                          QString::number(m_portConfig["remotePort"].toInt()), rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
