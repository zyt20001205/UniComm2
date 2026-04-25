#include "visa.h"

#include <QScopedValueRollback>

#include "globals.h"
#include "port/visa.h"
#include "util/suffixUtils.h"

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
        emit appendLog(LOG_INFO, QString("[%1]").arg(m_portConfig["portName"].toString()), "opened");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
        return true;
    }
    emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), "open failed");
    return false;
}

void Visa::close() {
    if (m_visa != VI_NULL) {
        ViStatus status = viClose(m_visa);
        emit refreshPort(m_portConfig["portName"].toString(), false);
        emit appendLog(LOG_INFO, QString("[%1]").arg(m_portConfig["portName"].toString()), "closed");
    }
}

void Visa::clear() {
}

bool Visa::write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
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

QByteArray Visa::read(const int length, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleRead(length, timeout);
}

QByteArray Visa::readUntil(const QByteArray &text, const int timeout, const QString &rxFormat) {
    QScopedValueRollback configRollback(m_portConfig);
    if (!rxFormat.isEmpty()) m_portConfig["rxFormat"] = rxFormat;
    return handleReadUntil(text, timeout);
}

// private
bool Visa::handleWrite(const QByteArray &f_txData) {
    // check port status
    if (m_visa == VI_NULL) {
        emit appendLog(LOG_ERROR, QString("[%1]").arg(m_portConfig["portName"].toString()), "not opened");
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

QByteArray Visa::handleReadUntil(const QByteArray &text, const int timeout) {
    return {};
}

void Visa::handleLog(const int type, const QByteArray &data) {
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
        emit appendLog(type, QString("[%1] ->").arg(m_portConfig["portName"].toString()), txMessage);
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
        emit appendLog(type, QString("[%1] <-").arg(m_portConfig["portName"].toString()), rxMessage);
    }
}
