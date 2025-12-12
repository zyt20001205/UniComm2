#include "luaModule/luaModbusRtu.h"

#include <sol/error.hpp>

#include "globals.h"
#include "suffix.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"

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
    txData += modbusCRC(txData);

    QByteArray rxData{};
    bool status = false;
    const int length = quantity * 2 + 5;

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &timeout, &length, &rxData] {
        status = port->write(txData, "raw", "null");
        rxData = port->read(timeout, length, "raw");
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("modbus rtu read holding registers failed");
    }
    if (static_cast<quint8>(rxData.at(0)) != slaveAddr) {
        throw sol::error("modbus rtu read holding registers slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(1)) != funcCode) {
        throw sol::error("modbus rtu read holding registers function code inconsistent");
    }
    const QByteArray checksum = rxData.right(2);
    rxData.chop(2);
    if (checksum != modbusCRC(rxData)) {
        throw sol::error("modbus rtu read holding registers checksum error");
    }
    const QByteArray regData = rxData.mid(3);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
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
    txData += modbusCRC(txData);

    QByteArray rxData{};
    bool status = false;

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &timeout] {
        status = port->write(txData, "raw", "null");
        rxData = port->read(timeout, 8, "raw");
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        throw sol::error("modbus rtu write multiple registers failed");
    }
    if (static_cast<quint8>(rxData.at(0)) != slaveAddr) {
        throw sol::error("modbus rtu write multiple registers slave address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(1)) != funcCode) {
        throw sol::error("modbus rtu write multiple registers function code inconsistent");
    }
    if (static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3)) != startAddr) {
        throw sol::error("modbus rtu write multiple registers start address inconsistent");
    }
    if (static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5)) != regCount) {
        throw sol::error("modbus rtu write multiple registers register count inconsistent");
    }
    const QByteArray checksum = rxData.right(2);
    rxData.chop(2);
    if (checksum != modbusCRC(rxData)) {
        throw sol::error("modbus rtu write multiple registers checksum error");
    }
}

// int lua_modbusAsciiReadHoldingRegisters(lua_State *L) {
//     // check arguments
//     if (lua_gettop(L) > 5)
//         luaL_error(L, "unexpected number of arguments");
//     // check arguments
//     const int param1 = static_cast<int>(luaL_checkinteger(L, 1));
//     const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
//     const int param3 = static_cast<int>(luaL_checkinteger(L, 3));
//     const int param4 = static_cast<int>(luaL_optinteger(L, 4, 1000));
//     const int param5 = static_cast<int>(luaL_optinteger(L, 5, -1));
//     // start operation
//     auto *portObject = g_port->portObject(param5);
//     const QString txSlaveAddr = QString("%1").arg(param1, 2, 10, QLatin1Char('0'));
//     const QString txFuncCode = "03";
//     const QString txStartAddr = QString("%1").arg(param2, 4, 10, QLatin1Char('0'));
//     const QString txQuantity = QString("%1").arg(param3, 4, 10, QLatin1Char('0'));
//     QString txText = ":";
//     txText.append(txSlaveAddr);
//     txText.append(txFuncCode);
//     txText.append(txStartAddr);
//     txText.append(txQuantity);
//     txText += modbusLRC(txText);
//     txText += "\r\n";
//     QMetaObject::invokeMethod(portObject, [portObject, txText] {
//         portObject->writeText(txText);
//     }, Qt::BlockingQueuedConnection);
//     QString rxText;
//     const int timeout = param4;
//     QMetaObject::invokeMethod(portObject, [&rxText, portObject, timeout] {
//         rxText = portObject->readText(timeout, 0); // length WIP
//     }, Qt::BlockingQueuedConnection);
//     if (rxText.at(0) != ":") {
//         luaL_error(L, "modbus ascii read holding registers header missing");
//         return 0;
//     }
//     if (const QString rxSlaveAddr = rxText.mid(1, 2); rxSlaveAddr != txSlaveAddr) {
//         luaL_error(L, "modbus ascii read holding registers slave address inconsistent");
//         return 0;
//     }
//     if (const QString rxFuncCode = rxText.mid(3, 2); rxFuncCode != txFuncCode) {
//         luaL_error(L, "modbus ascii read holding registers function code inconsistent");
//         return 0;
//     }
//     rxText.chop(2);
//     const QString rxChecksum = rxText.right(2);
//     rxText.chop(2);
//     if (rxChecksum != modbusLRC(rxText)) {
//         luaL_error(L, "modbus ascii read holding registers checksum error");
//         return 0;
//     }
//     const QString registerData = rxText.mid(7);
//     lua_pushstring(L, registerData.toUtf8().constData());
//     return 1;
// }
