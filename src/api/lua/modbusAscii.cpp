#include "api/lua/modbusAscii.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "utils/suffixUtils.h"

ModbusAscii::ModbusAscii(QObject *parent)
    : QObject(parent) {
}

std::string ModbusAscii::readHoldingRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const int quantity, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x03;
    QByteArray txData =
            ':'
            + QByteArray::number(slaveAddr, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(funcCode, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(startAddr, 16).rightJustified(4, '0').toUpper()
            + QByteArray::number(quantity, 16).rightJustified(4, '0').toUpper();
    const int length = quantity * 4 + 11;

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &length, &timeout] {
        if (!port->write(txData, "ascii", "modbus lrc")) {
            exception = "write failed";
            return;
        }

        rxData = port->read(length, timeout, "ascii");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (rxData.at(0) != ':')
        exception = "modbus ascii read holding registers comma missing";
    else if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr)
        exception = "modbus ascii read holding registers slave address inconsistent";
    else if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode)
        exception = "modbus ascii read holding registers function code inconsistent";
    else if (rxData.mid(5, 2).toUInt(nullptr, 16) != quantity * 2)
        exception = "modbus ascii read holding registers byte count inconsistent";
    else if (rxData.right(4) != modbusLRC(rxData.chopped(4)))
        exception = "modbus ascii read holding registers checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    const QByteArray regData = QByteArray::fromHex(rxData.chopped(4).mid(7));
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusAscii::writeSingleRegister(const std::string &portName, const int slaveAddr, const int regAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x06;
    QByteArray txData =
            ':'
            + QByteArray::number(slaveAddr, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(funcCode, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(regAddr, 16).rightJustified(4, '0').toUpper()
            + QByteArray::fromStdString(data);

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &timeout] {
        if (!port->write(txData, "ascii", "modbus lrc")) {
            exception = "write failed";
            return;
        }

        rxData = port->read(17, timeout, "ascii");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (rxData.at(0) != ':')
        exception = "modbus ascii write single register comma missing";
    else if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr)
        exception = "modbus ascii write single register slave address inconsistent";
    else if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode)
        exception = "modbus ascii write single register function code inconsistent";
    else if (rxData.mid(5, 4).toUInt(nullptr, 16) != regAddr)
        exception = "modbus ascii write single register register address inconsistent";
    else if (rxData.right(4) != modbusLRC(rxData.chopped(4)))
        exception = "modbus ascii write single register checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void ModbusAscii::writeMultipleRegisters(const std::string &portName, const int slaveAddr, const int startAddr, const std::string &data, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    QByteArray rxData{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    constexpr int funcCode = 0x10;
    const auto size = static_cast<qsizetype>(data.size() / 2);
    const int regCount = static_cast<int>(size) / 2;
    const int byteCount = static_cast<int>(size);
    QByteArray txData =
            ':'
            + QByteArray::number(slaveAddr, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(funcCode, 16).rightJustified(2, '0').toUpper()
            + QByteArray::number(startAddr, 16).rightJustified(4, '0').toUpper()
            + QByteArray::number(regCount, 16).rightJustified(4, '0').toUpper()
            + QByteArray::number(byteCount, 16).rightJustified(2, '0').toUpper()
            + QByteArray::fromStdString(data);

    QMetaObject::invokeMethod(port, [&exception, &rxData, &port, &txData, &timeout] {
        if (!port->write(txData, "ascii", "modbus lrc")) {
            exception = "write failed";
            return;
        }
        rxData = port->read(17, timeout, "ascii");
        if (rxData.isEmpty()) exception = "read timeout";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());

    if (rxData.at(0) != ':')
        exception = "modbus ascii write multiple registers comma missing";
    else if (rxData.mid(1, 2).toUInt(nullptr, 16) != slaveAddr)
        exception = "modbus ascii write multiple registers slave address inconsistent";
    else if (rxData.mid(3, 2).toUInt(nullptr, 16) != funcCode)
        exception = "modbus ascii write multiple registers function code inconsistent";
    else if (rxData.mid(5, 4).toUInt(nullptr, 16) != startAddr)
        exception = "modbus ascii write multiple registers register address inconsistent";
    else if (rxData.right(4) != modbusLRC(rxData.chopped(4)))
        exception = "modbus ascii write multiple registers checksum error";
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}
