#include "api/modbusRtu.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "util/uniCast.h"

// public
ModbusRtu::ModbusRtu(QObject *parent)
    : QObject(parent) {
}

void ModbusRtu::init(const std::string &portName, const int slaveAddr, const int timeout) {
    const auto port = g_port->m_portHash.constFind(QString::fromStdString(portName));
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");

    m_portName = portName;
    m_slaveAddr = slaveAddr;
    m_timeout = timeout;
    m_port = port.value();
}

std::string ModbusRtu::readHoldingRegisters(const int startAddr, const int quantity) const {
    Result result{};
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_slaveAddr));
    txData.append(FuncCode::ReadHoldingRegisters);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(quantity >> 8 & 0xFF));
    txData.append(static_cast<qint8>(quantity & 0xFF));
    const int length = quantity * 2 + 5;

    QMetaObject::invokeMethod(m_port, [this, &txData, &length] -> Result {
        if (!m_port->write(txData, "hex", "modbus crc")) return {{}, "write failed"};

        auto rxData = m_port->read(length, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::ReadHoldingRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (static_cast<quint8>(result.data.at(0)) != quantity * 2)
        throw sol::error(m_portName + ": modbus rtu read holding registers byte count inconsistent");

    const QByteArray regData = result.data.sliced(1);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusRtu::writeSingleRegister(const int regAddr, const std::string &data) const {
    Result result{};
    const auto value = QByteArray::fromHex(QByteArray::fromStdString(data));
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_slaveAddr));
    txData.append(FuncCode::WriteSingleRegister);
    txData.append(static_cast<qint8>(regAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regAddr & 0xFF));
    txData += value;

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "hex", "modbus crc")) return {{}, "write failed"};

        auto rxData = m_port->read(8, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::WriteSingleRegister, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus rtu write single register response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != regAddr)
        throw sol::error(m_portName + ": modbus rtu write single register register address inconsistent");
    if (result.data.sliced(2) != value)
        throw sol::error(m_portName + ": modbus rtu write single register register value inconsistent");
}

void ModbusRtu::writeMultipleRegisters(const int startAddr, const std::string &data) const {
    Result result{};
    const auto size = static_cast<qsizetype>(data.size() / 2);
    const int regCount = static_cast<int>(size) / 2;
    const int byteCount = static_cast<int>(size);
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_slaveAddr));
    txData.append(FuncCode::WriteMultipleRegisters);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(regCount >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regCount & 0xFF));
    txData.append(static_cast<qint8>(byteCount));
    txData += QByteArray::fromHex(QByteArray::fromStdString(data));

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "hex", "modbus crc")) return {{}, "write failed"};

        auto rxData = m_port->read(8, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::WriteMultipleRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus rtu write multiple registers response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != startAddr)
        throw sol::error(m_portName + ": modbus rtu write multiple registers start address inconsistent");
    if ((static_cast<quint8>(result.data.at(2)) << 8 | static_cast<quint8>(result.data.at(3))) != regCount)
        throw sol::error(m_portName + ": modbus rtu write multiple registers register count inconsistent");
}

// private
ModbusRtu::Result ModbusRtu::parser(const int funcCode, const QByteArray &rxData) const {
    if (rxData.isEmpty()) return {{}, "read timeout"};
    if (rxData.size() < 5) return {{}, "invalid modbus rtu response"};
    if (rxData.right(2) != uni_cast<ModbusCRC>(rxData.chopped(2)).value) return {{}, "modbus rtu checksum error"};
    if (static_cast<quint8>(rxData.at(0)) != m_slaveAddr) return {{}, "modbus rtu slave address inconsistent"};

    const auto rxFuncCode = static_cast<quint8>(rxData.at(1));
    if (rxFuncCode == (funcCode | FuncCode::ExceptionMask))
        return {{}, "modbus exception(" + QString::number(static_cast<quint8>(rxData.at(2))) + ")"};
    if (rxFuncCode != funcCode) return {{}, "modbus rtu function code inconsistent"};

    return {rxData.sliced(2, rxData.size() - 4), {}};
}
