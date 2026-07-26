#include "api/modbusTcp.h"

#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"

// public
ModbusTcp::ModbusTcp(QObject *parent)
    : QObject(parent) {
}

void ModbusTcp::init(const std::string &portName, const int transactionId, const int unitId, const int timeout) {
    const auto port = g_port->m_portHash.constFind(QString::fromStdString(portName));
    if (port == g_port->m_portHash.constEnd()) throw sol::error(portName + " does not exist");

    m_portName = portName;
    m_transactionId = transactionId;
    m_unitId = unitId;
    m_timeout = timeout;
    m_port = port.value();
}

std::string ModbusTcp::readHoldingRegisters(const int startAddr, const int quantity) const {
    Result result{};
    constexpr int protocolId = 0x00;
    constexpr int txLength = 6;
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(m_transactionId & 0xFF));
    txData.append(static_cast<qint8>(protocolId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(protocolId & 0xFF));
    txData.append(static_cast<qint8>(txLength >> 8 & 0xFF));
    txData.append(static_cast<qint8>(txLength & 0xFF));
    txData.append(static_cast<qint8>(m_unitId));
    txData.append(FuncCode::ReadHoldingRegisters);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(quantity >> 8 & 0xFF));
    txData.append(static_cast<qint8>(quantity & 0xFF));
    const int rxLength = quantity * 2 + 9;

    QMetaObject::invokeMethod(m_port, [this, &txData, rxLength] -> Result {
        if (!m_port->write(txData, "hex", "null")) return {{}, "write failed"};

        auto rxData = m_port->read(rxLength, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::ReadHoldingRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != quantity * 2 + 1 || static_cast<quint8>(result.data.at(0)) != quantity * 2)
        throw sol::error(m_portName + ": modbus tcp read holding registers byte count inconsistent");

    const QByteArray regData = result.data.sliced(1);
    return {regData.constData(), static_cast<std::string::size_type>(regData.size())};
}

void ModbusTcp::writeSingleRegister(const int regAddr, const std::string &data) const {
    Result result{};
    constexpr int protocolId = 0x00;
    constexpr int txLength = 6;
    const auto value = QByteArray::fromHex(QByteArray::fromStdString(data));
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(m_transactionId & 0xFF));
    txData.append(static_cast<qint8>(protocolId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(protocolId & 0xFF));
    txData.append(static_cast<qint8>(txLength >> 8 & 0xFF));
    txData.append(static_cast<qint8>(txLength & 0xFF));
    txData.append(static_cast<qint8>(m_unitId));
    txData.append(FuncCode::WriteSingleRegister);
    txData.append(static_cast<qint8>(regAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regAddr & 0xFF));
    txData += value;

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "hex", "null")) return {{}, "write failed"};

        auto rxData = m_port->read(12, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::WriteSingleRegister, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus tcp write single register response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != regAddr)
        throw sol::error(m_portName + ": modbus tcp write single register register address inconsistent");
    if (result.data.sliced(2) != value)
        throw sol::error(m_portName + ": modbus tcp write single register register value inconsistent");
}

void ModbusTcp::writeMultipleRegisters(const int startAddr, const std::string &data) const {
    Result result{};
    constexpr int protocolId = 0x00;
    const auto value = QByteArray::fromHex(QByteArray::fromStdString(data));
    const int txLength = static_cast<int>(value.size()) + 7;
    const int regCount = static_cast<int>(value.size()) / 2;
    const int byteCount = static_cast<int>(value.size());
    QByteArray txData{};
    txData.append(static_cast<qint8>(m_transactionId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(m_transactionId & 0xFF));
    txData.append(static_cast<qint8>(protocolId >> 8 & 0xFF));
    txData.append(static_cast<qint8>(protocolId & 0xFF));
    txData.append(static_cast<qint8>(txLength >> 8 & 0xFF));
    txData.append(static_cast<qint8>(txLength & 0xFF));
    txData.append(static_cast<qint8>(m_unitId));
    txData.append(FuncCode::WriteMultipleRegisters);
    txData.append(static_cast<qint8>(startAddr >> 8 & 0xFF));
    txData.append(static_cast<qint8>(startAddr & 0xFF));
    txData.append(static_cast<qint8>(regCount >> 8 & 0xFF));
    txData.append(static_cast<qint8>(regCount & 0xFF));
    txData.append(static_cast<qint8>(byteCount));
    txData += value;

    QMetaObject::invokeMethod(m_port, [this, &txData] -> Result {
        if (!m_port->write(txData, "hex", "null")) return {{}, "write failed"};

        auto rxData = m_port->read(12, m_timeout, "hex");
        if (rxData.isEmpty()) rxData = m_port->read(0, 0, "hex");

        return parser(FuncCode::WriteMultipleRegisters, rxData);
    }, Qt::BlockingQueuedConnection, &result);
    if (!result.exception.isEmpty()) throw sol::error(m_portName + ": " + result.exception.toStdString());

    if (result.data.size() != 4)
        throw sol::error(m_portName + ": invalid modbus tcp write multiple registers response");
    if ((static_cast<quint8>(result.data.at(0)) << 8 | static_cast<quint8>(result.data.at(1))) != startAddr)
        throw sol::error(m_portName + ": modbus tcp write multiple registers start address inconsistent");
    if ((static_cast<quint8>(result.data.at(2)) << 8 | static_cast<quint8>(result.data.at(3))) != regCount)
        throw sol::error(m_portName + ": modbus tcp write multiple registers register count inconsistent");
}

// private
ModbusTcp::Result ModbusTcp::parser(const int funcCode, const QByteArray &rxData) const {
    if (rxData.isEmpty()) return {{}, "read timeout"};
    if (rxData.size() < 9) return {{}, "invalid modbus tcp response"};

    const int transactionId = static_cast<quint8>(rxData.at(0)) << 8 | static_cast<quint8>(rxData.at(1));
    if (transactionId != m_transactionId) return {{}, "modbus tcp transaction id inconsistent"};

    const int protocolId = static_cast<quint8>(rxData.at(2)) << 8 | static_cast<quint8>(rxData.at(3));
    if (protocolId != 0) return {{}, "modbus tcp protocol id inconsistent"};

    const int length = static_cast<quint8>(rxData.at(4)) << 8 | static_cast<quint8>(rxData.at(5));
    if (length != rxData.size() - 6) return {{}, "modbus tcp length inconsistent"};
    if (static_cast<quint8>(rxData.at(6)) != m_unitId) return {{}, "modbus tcp unit id inconsistent"};

    const auto rxFuncCode = static_cast<quint8>(rxData.at(7));
    if (rxFuncCode == (funcCode | FuncCode::ExceptionMask))
        return {{}, "modbus exception(" + QString::number(static_cast<quint8>(rxData.at(8))) + ")"};
    if (rxFuncCode != funcCode) return {{}, "modbus tcp function code inconsistent"};

    return {rxData.sliced(8), {}};
}
