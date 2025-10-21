#ifndef UNICOMM_LUADATAPROCESS_H
#define UNICOMM_LUADATAPROCESS_H

#include <lua.hpp>

int lua_databaseList(lua_State *L);

int lua_databaseWrite(lua_State *L);

int lua_databaseClear(lua_State *L);

int lua_datatableList(lua_State *L);

int lua_datatableWrite(lua_State *L);

int lua_datatableClear(lua_State *L);

int lua_datatableExport(lua_State *L);

int lua_dataplotAppend(lua_State *L);

int lua_dataplotRemove(lua_State *L);

#endif //UNICOMM_LUADATAPROCESS_H
