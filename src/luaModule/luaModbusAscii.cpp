#include "luaModule/luaModbusAscii.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/suffixUtils.h"

LuaModbusAscii::LuaModbusAscii(QObject *parent)
    : QObject(parent) {
}

std::string LuaModbusAscii::readHoldingRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x03;
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(quantity >> 8 & 0xFF));
    txData.append(static_cast<qint8>(quantity & 0xFF));
    bool status = false;

    const int length = quantity * 4 + 11;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &length, &timeout, &rxData] {
        status = port->write(":" + txData.toHex().toUpper(), "ascii", "modbus lrc");
        rxData = port->read(length, timeout, "ascii");
    }, Qt::BlockingQueuedConnection);

    if (rxData.at(0) != ':') {
        throw sol::error(portName + ": modbus ascii read holding registers comma missing");
    }
    if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr) {
        throw sol::error(portName + ": modbus ascii read holding registers slave address inconsistent");
    }
    if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode) {
        throw sol::error(portName + ": modbus ascii read holding registers function code inconsistent");
    }
    if (rxData.mid(5, 2).toUInt(nullptr, 16) != quantity * 2) {
        throw sol::error(portName + ": modbus ascii read holding registers byte count inconsistent");
    }
    const QByteArray checksum = rxData.right(4);
    rxData.chop(4);
    if (checksum != modbusLRC(rxData)) {
        throw sol::error(portName + ": modbus ascii read holding registers checksum error");
    }
    const QByteArray regData = QByteArray::fromHex(rxData.mid(7));
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void LuaModbusAscii::writeSingleRegister(const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x06;
    const QByteArray regData(data.data(), static_cast<qsizetype>(data.size()));
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(regAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regAddr & 0xFF));
    txData += regData;
    bool status = false;

    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &timeout] {
        status = port->write(":" + txData.toHex().toUpper(), "ascii", "modbus lrc");
        rxData = port->read(8, timeout, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (rxData.at(0) != ':') {
        throw sol::error(portName + ": modbus ascii write single register comma missing");
    }
    if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr) {
        throw sol::error(portName + ": modbus ascii write single register slave address inconsistent");
    }
    if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode) {
        throw sol::error(portName + ": modbus ascii write single register function code inconsistent");
    }
    if (rxData.mid(5, 4).toUInt(nullptr, 16) != regAddr) {
        throw sol::error(portName + ": modbus ascii write single register register address inconsistent");
    }
    const QByteArray checksum = rxData.right(4);
    rxData.chop(4);
    if (checksum != modbusLRC(rxData)) {
        throw sol::error(portName + ": modbus ascii write single register checksum error");
    }
}

void LuaModbusAscii::writeMultipleRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x10;
    const auto size = static_cast<qsizetype>(data.size());
    const int regCount = static_cast<int>(size) / 2;
    const int byteCount = static_cast<int>(size);
    const QByteArray regData(data.data(), static_cast<qsizetype>(data.size()));
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(regCount >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regCount & 0xFF));
    txData.append(static_cast<qint8>(byteCount));
    txData += regData;
    bool status = false;

    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &timeout] {
        status = port->write(":" + txData.toHex().toUpper(), "ascii", "modbus lrc");
        rxData = port->read(8, timeout, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (rxData.at(0) != ':') {
        throw sol::error(portName + ": modbus ascii write multiple registers comma missing");
    }
    if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr) {
        throw sol::error(portName + ": modbus ascii write multiple registers slave address inconsistent");
    }
    if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode) {
        throw sol::error(portName + ": modbus ascii write multiple registers function code inconsistent");
    }
    if (rxData.mid(5, 4).toUInt(nullptr, 16) != startAddr) {
        throw sol::error(portName + ": modbus ascii write multiple registers register address inconsistent");
    }
    const QByteArray checksum = rxData.right(4);
    rxData.chop(4);
    if (checksum != modbusLRC(rxData)) {
        throw sol::error(portName + ": modbus ascii write multiple registers checksum error");
    }
}
