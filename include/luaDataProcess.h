#ifndef LUADATAPROCESS_H
#define LUADATAPROCESS_H

#include <lua.hpp>
#include "database.h"
#include "datatable.h"

extern Database *g_database;
extern Datatable *g_datatable;

int lua_databaseWrite(lua_State *L);

int lua_datatableWrite(lua_State *L);

#endif //LUADATAPROCESS_H