#ifndef LUADATAPROCESS_H
#define LUADATAPROCESS_H

#include <lua.hpp>

class Database;
extern Database *g_database;

class Datatable;
extern Datatable *g_datatable;

class Dataplot;
extern Dataplot *g_dataplot;

int lua_databaseWrite(lua_State *L);

int lua_databaseClear(lua_State *L);

int lua_datatableWrite(lua_State *L);

int lua_datatableClear(lua_State *L);

int lua_datatableExport(lua_State *L);

int lua_dataplotAppend(lua_State *L);

#endif //LUADATAPROCESS_H
