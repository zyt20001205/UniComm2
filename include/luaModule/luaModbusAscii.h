#ifndef UNICOMM_LUAMODBUSASCII_H
#define UNICOMM_LUAMODBUSASCII_H

#include <QObject>

class LuaModbusAscii final : public QObject {
    Q_OBJECT

public:
    explicit LuaModbusAscii(QObject *parent = nullptr);

    ~LuaModbusAscii() override = default;

    [[nodiscard]] static std::string readHoldingRegisters(const std::string &portName, int slaveAddr, int startAddr, int quantity, int timeout);

    static void writeSingleRegister(const std::string &portName, int slaveAddr, int regAddr, const std::string &data, int timeout);

    static void writeMultipleRegisters(const std::string &portName, int slaveAddr, int startAddr, const std::string &data, int timeout);
};

#endif //UNICOMM_LUAMODBUSASCII_H