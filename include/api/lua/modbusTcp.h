#ifndef UNICOMM_MODBUSTCP_H
#define UNICOMM_MODBUSTCP_H

#include <QObject>

class ModbusTcp final : public QObject {
    Q_OBJECT

public:
    explicit ModbusTcp(QObject *parent = nullptr);

    ~ModbusTcp() override = default;

    [[nodiscard]] static std::string readHoldingRegisters(const std::string &portName, int transactionId, int unitId, int startAddr, int quantity, int timeout);

    static void writeSingleRegister(const std::string &portName, int transactionId, int unitId, int regAddr, const std::string &data, int timeout);

    static void writeMultipleRegisters(const std::string &portName, int transactionId, int unitId, int startAddr, const std::string &data, int timeout);
};

#endif //UNICOMM_MODBUSTCP_H