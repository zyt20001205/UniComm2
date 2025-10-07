#ifndef LUAMODBUS_H
#define LUAMODBUS_H

#include <lua.hpp>

int lua_modbusRtuReadHoldingRegisters(lua_State *L);

int lua_modbusRtuWriteMultipleRegisters(lua_State *L);

int lua_modbusAsciiReadHoldingRegisters(lua_State *L);

#endif //LUAMODBUS_H