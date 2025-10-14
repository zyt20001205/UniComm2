#include "portModule/serialPort.h"

#include <QSerialPort>

#include "suffix.h"

// SerialPort public
SerialPort::SerialPort(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()),
      m_baudRate(portConfig["baudRate"].toInt()),
      m_dataBits(portConfig["dataBits"].toInt()),
      m_parity(portConfig["parity"].toInt()),
      m_stopBits(portConfig["stopBits"].toInt()),
      m_txFormat(portConfig["txFormat"].toString()),
      m_txSuffix(portConfig["txSuffix"].toString()),
      m_rxFormat(portConfig["rxFormat"].toString()) {
}

void SerialPort::reload(const QJsonObject &portConfig) {
    m_portName = portConfig["portName"].toString();
    m_baudRate = portConfig["baudRate"].toInt();
    m_dataBits = portConfig["dataBits"].toInt();
    m_parity = portConfig["parity"].toInt();
    m_stopBits = portConfig["stopBits"].toInt();
    m_txFormat = portConfig["txFormat"].toString();
    m_txSuffix = portConfig["txSuffix"].toString();
    m_rxFormat = portConfig["rxFormat"].toString();
}

QHash<QString, QVariant> SerialPort::info() {
    const bool status = m_serialPort->isOpen();
    const QString portName = m_portName;
    const QString baudRate = QString::number(m_baudRate);
    const QString dataBits = QString::number(m_dataBits);
    QString parity;
    switch (m_parity) {
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
        default: parity = "unknown";
    }
    QString stopBits;
    switch (m_stopBits) {
        case 1: stopBits = "1";
            break;
        case 3: stopBits = "1.5";
            break;
        case 2: stopBits = "2";
            break;
        default: stopBits = "unknown";
    }

    QHash<QString, QVariant> infoHash;
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
        connect(m_serialPort, &QSerialPort::readyRead, this, [this] { handleRead(0, 0); });
        connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
    }
    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_dataBits));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_parity));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_stopBits));
    // port open
    if (m_serialPort->open(QSerialPort::ReadWrite)) {
        emit appendLog(QString("%1 %2 %3").arg("serial port", m_portName, "opened"), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", m_portName, "opened");
        return true;
    }
    emit appendLog(QString("%1 %2 %3: %4").arg("serial port", m_portName, "open failed", m_serialPort->errorString()), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4: %5").arg(timestamp, "serial port", m_portName, "open failed", m_serialPort->errorString());
    return false;
}

void SerialPort::close() {
    // close port
    if (m_serialPort == nullptr) return;
    m_serialPort->close();
    emit appendLog(QString("%1 %2 %3").arg("serial port", m_portName, "closed"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3 %4").arg(timestamp, "serial port", m_portName, "closed");
}

bool SerialPort::writeText(const QString &txText) {
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

bool SerialPort::writeData(const QByteArray &txData) {
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

QString SerialPort::readText(const int timeout, const int length) {
    const QByteArray rxData = readData(timeout, length);
    if (rxData == "timeout") return "timeout";
    if (m_rxFormat == "hex") return m_rxBuffer.toHex().toUpper();
    if (m_rxFormat == "ascii") return QString::fromLatin1(m_rxBuffer);
    /* m_rxFormat == "utf-8" */
    return QString::fromUtf8(m_rxBuffer);
}

QByteArray SerialPort::readData(const int timeout, const int length) {
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
    }
    // sync mode
    else {
        disconnect(m_serialPort, &QSerialPort::readyRead, this, nullptr);
        rxData = handleRead(timeout, length);
        connect(m_serialPort, &QSerialPort::readyRead, this, [this] { handleRead(0, 0); });
    }
    return rxData;
}

// SerialPort private
bool SerialPort::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(QString("serial port %1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] serial port %2 is not opened").arg(timestamp, m_portName);
        return false;
    }
    m_serialPort->write(f_txData);
    // tx message reformat
    QString txMessage;
    // 1: encode tx message according to tx format
    if (m_txFormat == "hex") txMessage = f_txData.toHex(' ').toUpper();
    else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(f_txData);
    else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(f_txData);
    // 2: add port info
    txMessage = QString("[%1] -&gt; %2").arg(m_serialPort->portName(), txMessage);
    emit appendLog(txMessage, "tx");
    return true;
}

QByteArray SerialPort::handleRead(const int timeout, const int length) {
    // check port status
    if (m_serialPort == nullptr || !m_serialPort->isOpen()) {
        emit appendLog(QString("serial port %1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] serial port %2 is not opened").arg(timestamp, m_portName);
        return {};
    }
    QByteArray rxData = m_serialPort->readAll();
    if (timeout != 0 && length != 0) {
        int time = 0;
        while (rxData.size() != length) {
            if (m_serialPort->waitForReadyRead(10)) {
                rxData += m_serialPort->readAll();
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
    rxMessage = QString("[%1] &lt;- %2").arg(m_serialPort->portName(), rxMessage);
    emit appendLog(rxMessage, "rx");
    return rxData;
}

void SerialPort::handleError() {
}
