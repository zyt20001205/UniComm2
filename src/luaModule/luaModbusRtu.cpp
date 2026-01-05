#include "luaModule/luaModbusRtu.h"

#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/suffixUtils.h"

LuaModbusRtu::LuaModbusRtu(QObject *parent)
    : QObject(parent) {
}

std::string LuaModbusRtu::readHoldingRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const int timeout) {
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

    const int length = quantity * 2 + 5;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &timeout, &length, &rxData] {
        status = port->write(txData, "raw", "modbus crc");
        rxData = port->read(timeout, length, "raw");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (static_cast<quint8>(rxData.at(0)) != slaveAddr) {
        throw sol::error(portName + ": modbus rtu read holding registers slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(1)) != funcCode) {
        throw sol::error(portName + ": modbus rtu read holding registers function code inconsistent");
    }
    const QByteArray checksum = rxData.right(2);
    rxData.chop(2);
    if (checksum != modbusCRC(rxData)) {
        throw sol::error(portName + ": modbus rtu read holding registers checksum error");
    }
    const QByteArray regData = rxData.mid(3);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void LuaModbusRtu::writeSingleRegister(const std::string &portName, const int slaveAddr, const int regAddr, const std::string_view &data, const int timeout) {
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
        status = port->write(txData, "raw", "modbus crc");
        rxData = port->read(timeout, 8, "raw");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (static_cast<quint8>(rxData.at(0)) != slaveAddr) {
        throw sol::error(portName + ": modbus rtu write single register slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(1)) != funcCode) {
        throw sol::error(portName + ": modbus rtu write single register function code inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != regAddr) {
        throw sol::error(portName + ": modbus rtu write single register register address inconsistent");
    }
    const QByteArray checksum = rxData.right(2);
    rxData.chop(2);
    if (checksum != modbusCRC(rxData)) {
        throw sol::error(portName + ": modbus rtu write single register checksum error");
    }
}

void LuaModbusRtu::writeMultipleRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const std::string_view &data, const int timeout) {
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
        status = port->write(txData, "raw", "modbus crc");
        rxData = port->read(timeout, 8, "raw");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (static_cast<quint8>(rxData.at(0)) != slaveAddr) {
        throw sol::error(portName + ": modbus rtu write multiple registers slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(1)) != funcCode) {
        throw sol::error(portName + ": modbus rtu write multiple registers function code inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != startAddr) {
        throw sol::error(portName + ": modbus rtu write multiple registers start address inconsistent");
    }
    if ((static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5))) != regCount) {
        throw sol::error(portName + ": modbus rtu write multiple registers register count inconsistent");
    }
    const QByteArray checksum = rxData.right(2);
    rxData.chop(2);
    if (checksum != modbusCRC(rxData)) {
        throw sol::error(portName + ": modbus rtu write multiple registers checksum error");
    }
}
