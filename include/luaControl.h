#ifndef LUACONTROL_H
#define LUACONTROL_H

#include <QThread>
#include <lua.hpp>
#include <windows.h>

int lua_leftClick(lua_State *L);

int lua_leftDoubleClick(lua_State *L);

int lua_rightClick(lua_State *L);

int lua_rightDoubleClick(lua_State *L);

#endif //LUACONTROL_H