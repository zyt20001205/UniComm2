#include "portModule/serialPort.h"

#include <QElapsedTimer>
#include <QScopedValueRollback>
#include <QSerialPort>
#include <QThread>

#include "globals.h"
#include "suffix.h"

// SerialPort public
SerialPort::SerialPort(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
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

std::unordered_map<std::string, std::string> SerialPort::info() {
    const std::string status = m_serialPort && m_serialPort->isOpen() ? "opened" : "closed";
    const std::string portName = m_portConfig["portName"].toString().toStdString();
    const std::string baudRate = QString::number(m_portConfig["baudRate"].toInt()).toStdString();
    const std::string dataBits = QString::number(m_portConfig["dataBits"].toInt()).toStdString();
    std::string parity;
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
    std::string stopBits;
    switch (m_portConfig["stopBits"].toInt()) {
        case 1: stopBits = "1";
            break;
        case 3: stopBits = "1.5";
            break;
        case 2: stopBits = "2";
            break;
        default: stopBits = "?";
    }

    std::unordered_map<std::string, std::string> infoHash{};
    infoHash["status"] = status;
    infoHash["portName"] = portName;
    infoHash["baudRate"] = baudRate;
    infoHash["dataBits"] = dataBits;
    infoHash["parity"] = parity;
    infoHash["stopBits"] = stopBits;
    return infoHash;
}

bool SerialPort::open() {
    // port init
    if (m_serialPort == nullptr) {
        m_serialPort = new QSerialPort(this);
        connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPort::handleReadyRead);
        connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
    }
    m_serialPort->setPortName(m_portConfig["portName"].toString());
    m_serialPort->setBaudRate(m_portConfig["baudRate"].toInt());
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_portConfig["dataBits"].toInt()));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_portConfig["parity"].toInt()));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_portConfig["stopBits"].toInt()));
    // port open
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit refreshPort(m_portConfig["portName"].toString(), true);
        emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
        return true;
    }
    emit appendLog(QString("%1 open failed").arg(m_portConfig["portName"].toString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed").arg(timestamp, m_portConfig["portName"].toString());
    return false;
}

void SerialPort::close() {
    // port close
    if (m_serialPort == nullptr) return;
    m_serialPort->close();
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

bool SerialPort::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!txFormat.isEmpty()) m_portConfig["txFormat"] = txFormat;
    if (!txSuffix.isEmpty()) m_portConfig["txSuffix"] = txSuffix;
    // 1: remove space if tx format is hex
    QByteArray f_txData = txData;
    if (m_portConfig["txFormat"].toString() == "hex") f_txData = QByteArray::fromHex(txData);
    // 2: append suffix according to tx suffix
    if (m_portConfig["txSuffix"].toString() == "crlf") f_txData += "\r\n";
    else if (m_portConfig["txSuffix"].toString() == "crc16 modbus") f_txData += modbusCRC(f_txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray SerialPort::read(const int timeout, const int length, const QString &rxFormat) {
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
        m_syncMode = true;
        m_bufferSize = 0;
        rxData = handleRead(timeout, length);
        m_syncMode = false;
    }
    return rxData;
}

// SerialPort private
void SerialPort::handleReadyRead() {
    QByteArray rxData;
    if (m_syncMode) {
        const auto newBufferSize = m_serialPort->bytesAvailable();
        rxData = m_serialPort->peek(newBufferSize);
        handleLog("rx", rxData.mid(m_bufferSize));
        m_bufferSize = newBufferSize;
    } else {
        rxData = m_serialPort->readAll();
        handleLog("rx", rxData);
    }
    m_rxBuffer = rxData;
}

void SerialPort::handleError() {
    if (m_serialPort->error() == QSerialPort::NoError || m_serialPort->error() == QSerialPort::TimeoutError) return;
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 error: %2").arg(m_portConfig["portName"].toString(), m_serialPort->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 error: %3").arg(timestamp, m_portConfig["portName"].toString(), m_serialPort->errorString());
}

bool SerialPort::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    m_serialPort->write(f_txData);
    handleLog("tx", f_txData);
    return true;
}

QByteArray SerialPort::handleRead(const int timeout, const int length) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return {};
    }
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() <= timeout) {
        if (m_serialPort->bytesAvailable() == length) {
            QByteArray rxData = m_serialPort->readAll();
            return rxData;
        }
        m_serialPort->waitForReadyRead(10);
    }
    emit appendLog(QString("%1 timeout").arg(m_portConfig["portName"].toString()), "error");
    return {};
}

void SerialPort::handleLog(const QString &mode, const QByteArray &data) {
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
        txMessage = QString("[%1] -&gt; %2").arg(m_serialPort->portName(), txMessage);
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
        rxMessage = QString("[%1] &lt;- %2").arg(m_serialPort->portName(), rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
