#include "luaModule/modbusTcp.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

ModbusTcp::ModbusTcp(QObject *parent)
    : QObject(parent) {
}

std::string ModbusTcp::readHoldingRegisters(const std::string &portName, const int transactionId, const int unitId, const int startAddr, const int quantity, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int protocolId = 0x00;
    constexpr int txLength = 6;
    constexpr int funcCode = 0x03;
    QByteArray txData{};
    txData.append(static_cast<qint8>(transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(transactionId & 0xFF));
    txData.append(protocolId >> 8 & 0xFF);
    txData.append(protocolId & 0xFF);
    txData.append(txLength >> 8 & 0xFF);
    txData.append(txLength & 0xFF);
    txData.append(static_cast<qint8>(unitId));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(quantity >> 8 & 0xFF));
    txData.append(static_cast<qint8>(quantity & 0xFF));
    bool status = false;

    const int rxLength = quantity * 2 + 9;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxLength, &timeout, &rxData] {
        status = port->write(txData, "hex", "");
        rxData = port->read(rxLength, timeout, "hex");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if ((static_cast<quint8>(rxData.at(0)) << 8 | static_cast<quint8>(rxData.at(1))) != transactionId) {
        throw sol::error(portName + ": modbus tcp read holding registers transaction id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != protocolId) {
        throw sol::error(portName + ": modbus tcp read holding registers protocol id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5))) != quantity * 2 + 3) {
        throw sol::error(portName + ": modbus tcp read holding registers length inconsistent");
    }
    if (static_cast<quint8>(rxData.at(6)) != unitId) {
        throw sol::error(portName + ": modbus tcp read holding registers slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(7)) != funcCode) {
        throw sol::error(portName + ": modbus tcp read holding registers function code inconsistent");
    }
    if (static_cast<quint8>(rxData.at(8)) != quantity * 2) {
        throw sol::error(portName + ": modbus tcp read holding registers byte count inconsistent");
    }
    const QByteArray regData = rxData.mid(9);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusTcp::writeSingleRegister(const std::string &portName, const int transactionId, const int unitId, const int regAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int protocolId = 0x00;
    constexpr int txLength = 6;
    constexpr int funcCode = 0x06;
    const QByteArray regData = QByteArray::fromHex(QByteArray::fromStdString(data));
    QByteArray txData{};
    txData.append(static_cast<qint8>(transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(transactionId & 0xFF));
    txData.append(protocolId >> 8 & 0xFF);
    txData.append(protocolId & 0xFF);
    txData.append(txLength >> 8 & 0xFF);
    txData.append(txLength & 0xFF);
    txData.append(static_cast<qint8>(unitId));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(regAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regAddr & 0xFF));
    txData += regData;
    bool status = false;

    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &timeout] {
        status = port->write(txData, "hex", "null");
        rxData = port->read(12, timeout, "hex");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if ((static_cast<quint8>(rxData.at(0)) << 8 | static_cast<quint8>(rxData.at(1))) != transactionId) {
        throw sol::error(portName + ": modbus tcp write single register transaction id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != protocolId) {
        throw sol::error(portName + ": modbus tcp write single register protocol id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5))) != txLength) {
        throw sol::error(portName + ": modbus tcp write single register length inconsistent");
    }
    if (static_cast<quint8>(rxData.at(6)) != unitId) {
        throw sol::error(portName + ": modbus tcp write single register unit id inconsistent");
    }
    if (static_cast<quint8>(rxData.at(7)) != funcCode) {
        throw sol::error(portName + ": modbus tcp write single register function code inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(8)) << 8 | static_cast<quint8>(rxData.at(9))) != regAddr) {
        throw sol::error(portName + ": modbus tcp write single register register address inconsistent");
    }
}

void ModbusTcp::writeMultipleRegisters(const std::string &portName, const int transactionId, const int unitId, const int startAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int protocolId = 0x00;
    const auto size = static_cast<qsizetype>(data.size() / 2);
    const auto txLength = size + 7;
    constexpr int funcCode = 0x10;
    const int regCount = static_cast<int>(size) / 2;
    const int byteCount = static_cast<int>(size);
    const QByteArray regData = QByteArray::fromHex(QByteArray::fromStdString(data));
    QByteArray txData{};
    txData.append(static_cast<qint8>(transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(transactionId & 0xFF));
    txData.append(protocolId >> 8 & 0xFF);
    txData.append(protocolId & 0xFF);
    txData.append(static_cast<qint8>(txLength) >> 8 & 0xFF);
    txData.append(static_cast<qint8>(txLength) & 0xFF);
    txData.append(static_cast<qint8>(unitId));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(regCount >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regCount & 0xFF));
    txData.append(static_cast<qint8>(byteCount));
    txData += regData;
    bool status = false;

    constexpr int rxLength = 6;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &timeout] {
        status = port->write(txData, "hex", "null");
        rxData = port->read(12, timeout, "hex");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if ((static_cast<quint8>(rxData.at(0)) << 8 | static_cast<quint8>(rxData.at(1))) != transactionId) {
        throw sol::error(portName + ": modbus tcp write multiple registers transaction id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != protocolId) {
        throw sol::error(portName + ": modbus tcp write multiple registersr protocol id inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5))) != rxLength) {
        throw sol::error(portName + ": modbus tcp write multiple registers length inconsistent");
    }
    if (static_cast<quint8>(rxData.at(6)) != unitId) {
        throw sol::error(portName + ": modbus tcp write multiple registers unit id inconsistent");
    }
    if (static_cast<quint8>(rxData.at(7)) != funcCode) {
        throw sol::error(portName + ": modbus tcp write multiple registers function code inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(8)) << 8 | static_cast<quint8>(rxData.at(9))) != startAddr) {
        throw sol::error(portName + ": modbus tcp write multiple registers start address inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(10)) << 8 | static_cast<quint8>(rxData.at(11))) != regCount) {
        throw sol::error(portName + ": modbus tcp write multiple registers register count inconsistent");
    }
}
