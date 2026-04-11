#include "api/lua/modbusRtu.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "utils/suffixUtils.h"

ModbusRtu::ModbusRtu(QObject *parent)
    : QObject(parent) {
}

std::string ModbusRtu::readHoldingRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x03;
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(quantity >> 8 & 0xFF));
    txData.append(static_cast<qint8>(quantity & 0xFF));
    const int length = quantity * 2 + 5;

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &length, &timeout] {
        if (!port->write(txData, "hex", "modbus crc")) {
            exception = "write failed";
            return;
        }

        rxData = port->read(length, timeout, "hex");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (static_cast<quint8>(rxData.at(0)) != slaveAddr)
        exception = "modbus rtu read holding registers slave address inconsistent";
    else if (static_cast<quint8>(rxData.at(1)) != funcCode)
        exception = "modbus rtu read holding registers function code inconsistent";
    else if (static_cast<quint8>(rxData.at(2)) != quantity * 2)
        exception = "modbus rtu read holding registers byte count inconsistent";
    else if (rxData.right(2) != modbusCRC(rxData.chopped(2)))
        exception = "modbus rtu read holding registers checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    const QByteArray regData = rxData.chopped(2).mid(3);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusRtu::writeSingleRegister(const std::string &portName, const int slaveAddr, const int regAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x06;
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(regAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regAddr & 0xFF));
    txData += QByteArray::fromHex(QByteArray::fromStdString(data));

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &timeout] {
        if (!port->write(txData, "hex", "modbus crc")) {
            exception = "write failed";
            return;
        }

        rxData = port->read(8, timeout, "hex");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (static_cast<quint8>(rxData.at(0)) != slaveAddr)
        exception = "modbus rtu write single register slave address inconsistent";
    else if (static_cast<quint8>(rxData.at(1)) != funcCode)
        exception = "modbus rtu write single register function code inconsistent";
    else if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != regAddr)
        exception = "modbus rtu write single register register address inconsistent";
    else if (rxData.right(2) != modbusCRC(rxData.chopped(2)))
        exception = "modbus rtu write single register checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void ModbusRtu::writeMultipleRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x10;
    const auto size = static_cast<qsizetype>(data.size() / 2);
    const int regCount = static_cast<int>(size) / 2;
    const int byteCount = static_cast<int>(size);
    QByteArray txData{};
    txData.append(static_cast<qint8>(slaveAddr));
    txData.append(funcCode);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(regCount >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regCount & 0xFF));
    txData.append(static_cast<qint8>(byteCount));
    txData += QByteArray::fromHex(QByteArray::fromStdString(data));

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &timeout] {
        if (!port->write(txData, "hex", "modbus crc")) {
            exception = "write failed";
            return;
        }

        rxData = port->read(8, timeout, "hex");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (static_cast<quint8>(rxData.at(0)) != slaveAddr)
        exception = "modbus rtu write multiple registers slave address inconsistent";
    else if (static_cast<quint8>(rxData.at(1)) != funcCode)
        exception = "modbus rtu write multiple registers function code inconsistent";
    else if ((static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3))) != startAddr)
        exception = "modbus rtu write multiple registers start address inconsistent";
    else if ((static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5))) != regCount)
        exception = "modbus rtu write multiple registers register count inconsistent";
    else if (rxData.right(2) != modbusCRC(rxData.chopped(2)))
        exception = "modbus rtu write multiple registers checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}
