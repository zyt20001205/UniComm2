#include "visa.h"


#include "globals.h"
#include "portModule/visa.h"
#include "utils/suffixUtils.h"

// public
Visa::Visa(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig),
      m_buffer(portConfig["bufferSize"].toInt()) {
}

Visa::~Visa() {
    close();
}

int Visa::type() {
    return VISA;
}

QJsonObject Visa::config() {
    return m_portConfig;
}

QVariantHash Visa::info() {
    return {};
}

bool Visa::open() {
    ViStatus status = viOpen(g_rm, m_portConfig["portName"].toString().toUtf8().constData(), VI_NULL, VI_NULL, &m_visa);
    if (status == VI_SUCCESS) {
        status = viSetAttribute(m_visa, VI_ATTR_TMO_VALUE, 5000);
        emit refreshPort(m_portConfig["portName"].toString(), true);
        emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), LOG_INFO);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
        return true;
    }
    emit appendLog(QString("%1 open failed").arg(m_portConfig["portName"].toString()), LOG_ERROR);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed").arg(timestamp, m_portConfig["portName"].toString());
    return false;
}

void Visa::close() {
    if (m_visa != VI_NULL) {
        ViStatus status = viClose(m_visa);
        emit refreshPort(m_portConfig["portName"].toString(), false);
        emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), LOG_INFO);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
    }
}

void Visa::clear() {
}

bool Visa::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    const QString format = txFormat.isEmpty() ? m_portConfig["txFormat"].toString() : txFormat;
    const QString suffix = txSuffix.isEmpty() ? m_portConfig["txSuffix"].toString() : txSuffix;
    // 1: remove space if tx format is hex
    QByteArray f_txData = txData;
    if (format == "hex") f_txData = QByteArray::fromHex(txData);
    // 2: append suffix according to tx suffix
    if (suffix == "crlf") f_txData += "\r\n";
    else if (suffix == "modbus crc") f_txData += modbusCRC(f_txData);
    else if (suffix == "modbus lrc") f_txData += modbusLRC(f_txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray Visa::read(const int length, const int timeout, const QString &rxFormat) {
    return handleRead(length, timeout);
}

// private
bool Visa::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_visa == VI_NULL) {
        emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), LOG_ERROR);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
        return false;
    }
    ViUInt32 retCount;
    const ViStatus status = viWrite(m_visa, (ViBuf) f_txData.constData(), f_txData.size(), &retCount);
    handleLog(LOG_TX, f_txData);
    if (status == VI_SUCCESS) {
        return true;
    }
    return false;
}

QByteArray Visa::handleRead(const int length, const int timeout) {
    // // check port status
    // if (m_Visa == nullptr || !m_Visa->isOpen()) {
    //     emit appendLog(QString("%1 is not opened").arg(m_portConfig["portName"].toString()), "error");
    //     // logging
    //     QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    //     qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portConfig["portName"].toString());
    //     return {};
    // }
    // QElapsedTimer timer;
    // timer.start();
    // while (timer.elapsed() <= timeout) {
    //     if (m_Visa->bytesAvailable() == length) {
    //         QByteArray rxData = m_Visa->readAll();
    //         return rxData;
    //     }
    //     m_Visa->waitForReadyRead(10);
    // }
    // emit appendLog(QString("%1 timeout").arg(m_portConfig["portName"].toString()), "error");
    return {};
}

void Visa::handleLog(const int type, const QByteArray &data) {
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
        txMessage = QString("[%1] -&gt; %2").arg(m_portConfig["portName"].toString(), txMessage);
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
        rxMessage = QString("[%1] &lt;- %2").arg(m_portConfig["portName"].toString(), rxMessage);
        emit appendLog(rxMessage, type);
    }
}
