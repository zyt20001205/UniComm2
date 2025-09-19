#ifndef LUADATAPROCESS_H
#define LUADATAPROCESS_H

#include <lua.hpp>
#include "database.h"
#include "datatable.h"
#include "dataplot.h"

extern Database *g_database;
extern Datatable *g_datatable;
extern Dataplot *g_dataplot;

int lua_databaseWrite(lua_State *L);

int lua_databaseClear(lua_State *L);

int lua_datatableWrite(lua_State *L);

int lua_datatableClear(lua_State *L);

int lua_dataplotAppend(lua_State *L);

#endif //LUADATAPROCESS_H
