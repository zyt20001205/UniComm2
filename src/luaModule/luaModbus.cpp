#include "luaModule/luaModbus.h"

#include "globals.h"
#include "suffix.h"
#include "portModule/portModule.h"
#include "portModule/basePort.h"

int lua_modbusRtuReadHoldingRegisters(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 4 && lua_gettop(L) != 5)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    const int param3 = static_cast<int>(luaL_checkinteger(L, 3));
    const int param4 = static_cast<int>(luaL_checkinteger(L, 4));
    const int param5 = static_cast<int>(luaL_optinteger(L, 5, 1000));
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        auto *portObject = g_port->m_portHash[portName];
        const int txSlaveAddr = param2;
        constexpr int txFuncCode = 0x03;
        const int txStartAddr = param3;
        const int txQuantity = param4;
        const int rxLength = txQuantity * 2 + 5;
        const int rxTimeout = param5;

        QByteArray txData{};
        txData.append(static_cast<char>(txSlaveAddr));
        txData.append(txFuncCode);
        txData.append(static_cast<char>(txStartAddr >> 8 & 0xFF));
        txData.append(static_cast<char>(txStartAddr & 0xFF));
        txData.append(static_cast<char>(txQuantity >> 8 & 0xFF));
        txData.append(static_cast<char>(txQuantity & 0xFF));
        txData += modbusCRC(txData);
        QByteArray rxData{};
        bool status = false;
        QMetaObject::invokeMethod(portObject, [portObject, txData, &rxData, rxTimeout, rxLength, &status] {
            status = portObject->writeData(txData);
            rxData = portObject->readData(rxTimeout, rxLength);
        }, Qt::BlockingQueuedConnection);
        if (!status) {
            luaL_error(L, "modbus rtu read holding registers failed");
        }
        if (rxData.length() != rxLength) {
            luaL_error(L, "modbus rtu read holding registers wrong length");
            qDebug() << rxData;
            return 0;
        }
        if (const uint8_t rxSlaveAddr = rxData.at(0); rxSlaveAddr != txSlaveAddr) {
            luaL_error(L, "modbus rtu read holding registers slave address inconsistent");
            return 0;
        }
        if (const uint8_t rxFuncCode = rxData.at(1); rxFuncCode != txFuncCode) {
            luaL_error(L, "modbus rtu read holding registers function code inconsistent");
            return 0;
        }
        const QByteArray rxChecksum = rxData.right(2);
        rxData.chop(2);
        if (rxChecksum != modbusCRC(rxData)) {
            luaL_error(L, "modbus rtu read holding registers checksum error");
            return 0;
        }
        const QByteArray registerData = rxData.mid(3);
        lua_pushlstring(L, registerData.constData(), registerData.size());
        return 1;
    }
}

int lua_modbusRtuWriteMultipleRegisters(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 4 && lua_gettop(L) != 5)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = static_cast<int>(luaL_checkinteger(L, 2));
    const int param3 = static_cast<int>(luaL_checkinteger(L, 3));
    size_t len4;
    const char *param4 = luaL_checklstring(L, 4, &len4);
    const int param5 = static_cast<int>(luaL_optinteger(L, 5, 1000));
    // start operation
    const QString portName = QString::fromUtf8(param1);
    if (!g_port->m_portHash.contains(portName)) {
        luaL_error(L, "port '%s' does not exist", portName.toUtf8().constData());
    } else {
        auto *portObject = g_port->m_portHash[portName];
        const int txSlaveAddr = param2;
        constexpr int txFuncCode = 0x10;
        const int txStartAddr = param3;
        const QByteArray txRegData(param4, static_cast<qsizetype>(len4));
        const int rxTimeout = param5;
        const int txRegCount = static_cast<int>(len4) / 2;
        const int txByteCount = static_cast<int>(len4);
        constexpr int rxLength = 8;

        QByteArray txData{};
        txData.append(static_cast<uint8_t>(txSlaveAddr));
        txData.append(txFuncCode);
        txData.append(static_cast<uint8_t>(txStartAddr >> 8 & 0xFF));
        txData.append(static_cast<uint8_t>(txStartAddr & 0xFF));
        txData.append(static_cast<uint8_t>(txRegCount >> 8 & 0xFF));
        txData.append(static_cast<uint8_t>(txRegCount & 0xFF));
        txData.append(static_cast<uint8_t>(txByteCount));
        txData += txRegData;
        txData += modbusCRC(txData);
        QByteArray rxData{};
        bool status = false;
        QMetaObject::invokeMethod(portObject, [portObject, txData, &rxData, rxTimeout, &status] {
            status = portObject->writeData(txData);
            rxData = portObject->readData(rxTimeout, 8);
        }, Qt::BlockingQueuedConnection);
        if (!status) {
            luaL_error(L, "modbus rtu write multiple registers failed");
        }
        if (rxData.length() != rxLength) {
            qDebug() << rxData;
            luaL_error(L, "modbus rtu write multiple registers wrong length");
            return 0;
        }
        if (const uint8_t rxSlaveAddr = static_cast<uint8_t>(rxData.at(0)); rxSlaveAddr != txSlaveAddr) {
            luaL_error(L, "modbus rtu write multiple registers slave address inconsistent");
        }
        if (const uint8_t rxFuncCode = static_cast<uint8_t>(rxData.at(1)); rxFuncCode != txFuncCode) {
            luaL_error(L, "modbus rtu write multiple registers function code inconsistent");
        }
        if (const uint16_t rxStartAddr = static_cast<uint8_t>(rxData.at(2)) << 8 | static_cast<uint8_t>(rxData.at(3)); rxStartAddr != txStartAddr) {
            luaL_error(L, "modbus rtu write multiple registers start address inconsistent");
        }
        if (const uint16_t rxRegCount = static_cast<uint8_t>(rxData.at(4)) << 8 | static_cast<uint8_t>(rxData.at(5)); rxRegCount != txRegCount) {
            luaL_error(L, "modbus rtu write multiple registers register count inconsistent");
        }
        const QByteArray rxChecksum = rxData.right(2);
        rxData.chop(2);
        if (rxChecksum != modbusCRC(rxData)) {
            luaL_error(L, "modbus rtu write multiple registers checksum error");
        }
        return 0;
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
