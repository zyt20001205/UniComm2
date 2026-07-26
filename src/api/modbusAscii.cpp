#include "api/modbusAscii.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
ModbusAscii::ModbusAscii(QObject *parent)
    : QObject(parent) {
}

void ModbusAscii::init(const std::string &portName, const int slaveAddr, const int timeout) {
    const auto port = g_port->m_portHash.constFind(QString::fromStdString(portName));
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");

    m_portName = portName;
    m_slaveAddr = slaveAddr;
    m_timeout = timeout;
    m_port = port.value();
}

std::string ModbusAscii::readHoldingRegisters(const int startAddr, const int quantity) const {
    Result result{};
    QByteArray txData = ":";
    txData += QByteArray::number(m_slaveAddr, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(FuncCode::ReadHoldingRegisters, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(startAddr, 16).rightJustified(4, '0').toUpper();
    txData += QByteArray::number(quantity, 16).rightJustified(4, '0').toUpper();
    const int length = quantity * 4 + 11;

    QMetaObject::invokeMethod(m_port, [this, &txData, length] -> Result {
        if (!m_port->write(txData, "ascii", "modbus lrc")) return {{}, "write failed"};

        auto rxData = m_port->read(length, m_timeout, "ascii");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "ascii");

        return parser(FuncCode::ReadHoldingRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != quantity * 2 + 1 || static_cast<quint8>(result.data.at(0)) != quantity * 2)
        throw sol::error(m_portName + ": modbus ascii read holding registers byte count inconsistent");

    const QByteArray regData = result.data.sliced(1);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusAscii::writeSingleRegister(const int regAddr, const std::string &data) const {
    Result result{};
    const auto value = QByteArray::fromHex(QByteArray::fromStdString(data));
    QByteArray txData = ":";
    txData += QByteArray::number(m_slaveAddr, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(FuncCode::WriteSingleRegister, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(regAddr, 16).rightJustified(4, '0').toUpper();
    txData += value.toHex().toUpper();

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "ascii", "modbus lrc")) return {{}, "write failed"};

        auto rxData = m_port->read(17, m_timeout, "ascii");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "ascii");

        return parser(FuncCode::WriteSingleRegister, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus ascii write single register response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != regAddr)
        throw sol::error(m_portName + ": modbus ascii write single register register address inconsistent");
    if (result.data.sliced(2) != value)
        throw sol::error(m_portName + ": modbus ascii write single register register value inconsistent");
}

void ModbusAscii::writeMultipleRegisters(const int startAddr, const std::string &data) const {
    Result result{};
    const auto value = QByteArray::fromHex(QByteArray::fromStdString(data));
    const int regCount = static_cast<int>(value.size()) / 2;
    const int byteCount = static_cast<int>(value.size());
    QByteArray txData = ":";
    txData += QByteArray::number(m_slaveAddr, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(FuncCode::WriteMultipleRegisters, 16).rightJustified(2, '0').toUpper();
    txData += QByteArray::number(startAddr, 16).rightJustified(4, '0').toUpper();
    txData += QByteArray::number(regCount, 16).rightJustified(4, '0').toUpper();
    txData += QByteArray::number(byteCount, 16).rightJustified(2, '0').toUpper();
    txData += value.toHex().toUpper();

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "ascii", "modbus lrc")) return {{}, "write failed"};

        auto rxData = m_port->read(17, m_timeout, "ascii");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "ascii");

        return parser(FuncCode::WriteMultipleRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus ascii write multiple registers response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != startAddr)
        throw sol::error(m_portName + ": modbus ascii write multiple registers start address inconsistent");
    if ((static_cast<quint8>(result.data.at(2)) << 8 | static_cast<quint8>(result.data.at(3))) != regCount)
        throw sol::error(m_portName + ": modbus ascii write multiple registers register count inconsistent");
}

// private
ModbusAscii::Result ModbusAscii::parser(const int funcCode, const QByteArray &rxData) const {
    if (rxData.isEmpty()) return {{}, "read timeout"};
    if (rxData.size() < 11) return {{}, "invalid modbus ascii response"};
    if (rxData.at(0) != ':') return {{}, "modbus ascii colon missing"};
    if (rxData.right(4) != uni_cast<ModbusLRC>(rxData.chopped(4)).value) return {{}, "modbus ascii checksum error"};
    if (rxData.sliced(1, 2).toUInt(nullptr, 16) != m_slaveAddr) return {{}, "modbus ascii slave address inconsistent"};

    const auto rxFuncCode = rxData.sliced(3, 2).toUInt(nullptr, 16);
    if (rxFuncCode == (funcCode | FuncCode::ExceptionMask))
        return {{}, "modbus exception(" + QString::number(rxData.sliced(5, 2).toUInt(nullptr, 16)) + ")"};
    if (rxFuncCode != funcCode) return {{}, "modbus ascii function code inconsistent"};

    return {QByteArray::fromHex(rxData.sliced(5, rxData.size() - 9)), {}};
}
