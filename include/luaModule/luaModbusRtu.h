#ifndef UNICOMM_LUAMODBUS_H
#define UNICOMM_LUAMODBUS_H

#include <QObject>

class LuaModbusRtu final : public QObject {
    Q_OBJECT

public:
    explicit LuaModbusRtu(QObject *parent = nullptr);

    ~LuaModbusRtu() override = default;

    std::string readHoldingRegisters(const std::string &portName, int slaveAddr, int startAddr, int quantity, int timeout);

    void writeMultipleRegisters(const std::string &portName, int slaveAddr, int startAddr, const std::string_view &data, int timeout);
};

// int lua_modbusAsciiReadHoldingRegisters(lua_State *L);

#endif //UNICOMM_LUAMODBUS_H