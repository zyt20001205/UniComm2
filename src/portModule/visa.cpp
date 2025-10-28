#include "visa.h"

#include <QScopedValueRollback>

#include "globals.h"
#include "portModule/visa.h"
#include "suffix.h"

// Visa public
Visa::Visa(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()) {
}

void Visa::reload(const QJsonObject &portConfig) {
    m_portName = portConfig["portName"].toString();
}

QVariantMap Visa::info() {
    return {};
}

bool Visa::open() {
    ViStatus status = viOpen(g_rm, m_portName.toUtf8().constData(), VI_NULL, VI_NULL, &m_visa);
    if (status == VI_SUCCESS) {
        status = viSetAttribute(m_visa, VI_ATTR_TMO_VALUE, 5000);
        emit togglePort(true);
        emit appendLog(QString("%1 opened").arg(m_portName), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portName);
        return true;
    }
    emit appendLog(QString("%1 open failed").arg(m_portName), "error");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 open failed").arg(timestamp, m_portName);
    return false;
}

void Visa::close() {
    if (m_visa != VI_NULL) {
        ViStatus status = viClose(m_visa);
        emit togglePort(false);
        emit appendLog(QString("%1 closed").arg(m_portName), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portName);
    }
}

bool Visa::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
    QScopedValueRollback txFormatRollback(m_txFormat);
    QScopedValueRollback txSuffixRollback(m_txSuffix);
    if (!txFormat.isEmpty()) m_txFormat = txFormat;
    if (!txSuffix.isEmpty()) m_txSuffix = txSuffix;
    // 1: remove space if tx format is hex
    QByteArray f_txData = txData;
    if (m_txFormat == "hex") f_txData = QByteArray::fromHex(txData);
    // 2: append suffix according to tx suffix
    if (m_txSuffix == "crlf") f_txData += "\r\n";
    else if (m_txSuffix == "crc16 modbus") f_txData += modbusCRC(f_txData);
    // call handle write
    return handleWrite(f_txData);
}

QByteArray Visa::read(const int timeout, const int length, const QString &rxFormat) {
    QScopedValueRollback rxFormatRollback(m_rxFormat);
    if (!rxFormat.isEmpty()) m_rxFormat = rxFormat;
    QByteArray rxData;
    // async mode
    if (timeout == 0) {
        rxData = m_rxBuffer;
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

// Visa private
bool Visa::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_visa == VI_NULL) {
        emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
        return false;
    }
    ViUInt32 retCount;
    const ViStatus status = viWrite(m_visa, (ViBuf) f_txData.constData(), f_txData.size(), &retCount);
    handleLog("tx", f_txData);
    if (status == VI_SUCCESS) {
        return true;
    }
    return false;
}

QByteArray Visa::handleRead(const int timeout, const int length) {
    // // check port status
    // if (m_Visa == nullptr || !m_Visa->isOpen()) {
    //     emit appendLog(QString("%1 is not opened").arg(m_portName), "error");
    //     // logging
    //     QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    //     qDebug() << QString("[%1] %2 is not opened").arg(timestamp, m_portName);
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
    // emit appendLog(QString("%1 timeout").arg(m_portName), "error");
    return {};
}

void Visa::handleLog(const QString &mode, const QByteArray &data) {
    if (mode == "tx") {
        // tx message reformat
        QString txMessage;
        // 1: encode tx message according to tx format
        if (m_txFormat == "raw") {
            txMessage.reserve(data.size() * 4);
            for (const char c: data) {
                txMessage += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
            }
        } else if (m_txFormat == "hex") txMessage = data.toHex(' ').toUpper();
        else if (m_txFormat == "ascii") txMessage = QString::fromLatin1(data);
        else /* m_txFormat == "utf-8" */ txMessage = QString::fromUtf8(data);
        // 2: add port info
        txMessage = QString("[%1] -&gt; %2").arg(m_portName, txMessage);
        emit appendLog(txMessage, mode);
    } else {
        // rx message reformat
        QString rxMessage;
        // 1: encode rx message according to rx format
        if (m_rxFormat == "raw") {
            rxMessage.reserve(data.size() * 4);
            for (const char c: data) {
                rxMessage += QString("\\x%1").arg(static_cast<quint8>(c), 2, 16, QChar('0'));
            }
        } else if (m_rxFormat == "hex") rxMessage = data.toHex(' ').toUpper();
        else if (m_rxFormat == "ascii") rxMessage = QString::fromLatin1(data);
        else /* m_rxFormat == "utf-8" */ rxMessage = QString::fromUtf8(data);
        // 2: add port info
        rxMessage = QString("[%1] &lt;- %2").arg(m_portName, rxMessage);
        emit appendLog(rxMessage, mode);
    }
}
