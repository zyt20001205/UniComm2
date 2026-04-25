#include "port/serialPort.h"

#include <QDeadlineTimer>
#include <QScopedValueRollback>
#include <QSerialPort>
#include <QThread>

#include "globals.h"
#include "util/suffixUtils.h"

// public
SerialPort::SerialPort(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(portConfig["bufferSize"].toInt()) {
}

SerialPort::~SerialPort() {
    close();
}

int SerialPort::type() {
    return SERIALPORT;
}

QJsonObject SerialPort::config() {
    return m_portConfig;
}

QVariantHash SerialPort::info() {
    const QString status = m_serialPort && m_serialPort->isOpen() ? "opened" : "closed";
    const auto portName = m_portConfig["portName"].toString();
    const auto baudRate = QString::number(m_portConfig["baudRate"].toInt());
    const auto dataBits = QString::number(m_portConfig["dataBits"].toInt());
    QString parity{};
    switch (m_portConfig["parity"].toInt()) {
        case 0: parity = "no";
            break;
        case 2: parity = "even";
            break;
        case 3: parity = "odd";
            break;
        case 4: parity = "space";
            break;
        case 5: parity = "mark";
            break;
        default: parity = "?";
    }
    QString stopBits{};
    switch (m_portConfig["stopBits"].toInt()) {
        case 1: stopBits = "1";
            break;
        case 3: stopBits = "1.5";
            break;
        case 2: stopBits = "2";
            break;
        default: stopBits = "?";
    }
    const auto bufferSize = QString::number(m_portConfig["bufferSize"].toInt());
    const auto bufferUsed = QString::number(m_buffer.used());

    QVariantHash infoHash = {
        {"status", status},
        {"portName", portName},
        {"baudRate", baudRate},
        {"dataBits", dataBits},
        {"parity", parity},
        {"stopBits", stopBits},
        {"bufferSize", bufferSize},
        {"bufferUsed", bufferUsed}
    };
    return infoHash;
}

bool SerialPort::open() {
    // status check
    if (m_serialPort == nullptr) {
        m_serialPort = new QSerialPort(this);
        connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPort::handleReadyRead);
        connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
    }
    if (m_serialPort->isOpen()) return true;
    // port open
    m_serialPort->setPortName(m_portConfig["portName"].toString());
    m_serialPort->setBaudRate(m_portConfig["baudRate"].toInt());
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_portConfig["dataBits"].toInt()));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_portConfig["parity"].toInt()));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_portConfig["stopBits"].toInt()));
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit refreshPort(m_portConfig["portName"].toString(), true);
        emit appendLog(LOG_INFO, QString("[%1]").arg(m_portConfig["portName"].toString()), "opened");
        return true;
    }
    emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("open failed: %1").arg(m_serialPort->errorString()));
    return false;
}

void SerialPort::close() {
    // status check
    if (m_serialPort == nullptr) return;
    // port close
    m_serialPort->close();
    clear();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(LOG_INFO, QString("[%1]").arg(m_portConfig["portName"].toString()), "closed");
}

void SerialPort::clear() {
    m_buffer.clear();
}

bool SerialPort::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

QByteArray SerialPort::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

QByteArray SerialPort::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout);
}

// private
void SerialPort::handleReadyRead() {
    const auto rxData = m_serialPort->readAll();
    m_buffer.write(rxData);
    handleLog(LOG_RX, rxData);
}

void SerialPort::handleError() {
    if (m_serialPort->error() == QSerialPort::NoError || m_serialPort->error() == QSerialPort::TimeoutError) return;
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), QString("%1").arg(m_serialPort->errorString()));
}

bool SerialPort::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return false;
    }
    m_serialPort->write(f_txData);
    handleLog(LOG_TX, f_txData);
    return true;
}

QByteArray SerialPort::handleRead(const int length, const int timeout) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.used() < length) {
        if (deadline.hasExpired()) break;
        m_serialPort->waitForReadyRead(10);
    }
    return m_buffer.read(length);
}

QByteArray SerialPort::handleReadUntil(const QByteArray &text, const int timeout) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
        return {};
    }
    const QDeadlineTimer deadline(timeout);
    while (m_buffer.distance(text) == -1) {
        if (deadline.hasExpired()) break;
        m_serialPort->waitForReadyRead(10);
    }
    return m_buffer.read(m_buffer.distance(text));
}

void SerialPort::handleLog(const int type, const QByteArray &data) {
    if (type == LOG_TX) {
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
        emit appendLog(type, QString("[%1] -&gt;").arg(m_serialPort->portName()), txMessage);
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
        emit appendLog(type, QString("[%1] &lt;-").arg(m_serialPort->portName()), rxMessage);
    }
}
