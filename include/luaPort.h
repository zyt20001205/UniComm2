#ifndef LUAPORT_H
#define LUAPORT_H

#include <lua.hpp>
#include "log.h"
#include "port.h"

extern Log *g_log;
extern Port *g_port;

int lua_portOpen(lua_State *L);

int lua_portClose(lua_State *L);

int lua_portInfo(lua_State *L);

int lua_portWriteText(lua_State *L);

int lua_portWriteData(lua_State *L);

int lua_portReadText(lua_State *L);

int lua_portReadData(lua_State *L);

#endif //LUAPORT_H
