#ifndef UNICOMM_LUAMODBUSRTU_H
#define UNICOMM_LUAMODBUSRTU_H

#include <QObject>

class LuaModbusRtu final : public QObject {
    Q_OBJECT

public:
    explicit LuaModbusRtu(QObject *parent = nullptr);

    ~LuaModbusRtu() override = default;

    [[nodiscard]] static std::string readHoldingRegisters(const std::string &portName, int slaveAddr, int startAddr, int quantity, int timeout);

    static void writeSingleRegister(const std::string &portName, int slaveAddr, int regAddr, const std::string_view &data, int timeout);

    static void writeMultipleRegisters(const std::string &portName, int slaveAddr, int startAddr, const std::string_view &data, int timeout);
};

#endif //UNICOMM_LUAMODBUSRTU_H