#include "luaModule/luaDataProcess.h"

#include "dataModule/databaseModule.h"
#include "dataModule/datatableModule.h"
#include "dataModule/dataplotModule.h"
#include "globals.h"

int lua_databaseWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    // start operation
    bool status = false;
    const QString key = param1;
    const QString value = param2;
    QMetaObject::invokeMethod(g_mainWindow, [&status, key, value] {
        status = g_database->databaseWrite(key, value);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        luaL_error(L, "key not found in database");
    }
    return 0;
}

int lua_databaseClear(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 0)
        luaL_error(L, "unexpected number of arguments");
    // start operation
    QMetaObject::invokeMethod(g_mainWindow, [] {
        g_database->databaseClear();
    }, Qt::QueuedConnection);
    return 0;
}

int lua_datatableWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    // start operation
    bool status = false;
    const QString key = param1;
    const QString value = param2;
    QMetaObject::invokeMethod(g_mainWindow, [&status, key, value] {
        status = g_datatable->datatableWrite(key, value);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        luaL_error(L, "key not found in datatable");
    }
    return 0;
}

int lua_datatableClear(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 0 && lua_gettop(L) != 1)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_optstring(L, 1, "");
    // start operation
    bool status = false;
    const QString key = param1;
    QMetaObject::invokeMethod(g_mainWindow, [&status, key] {
        status = g_datatable->datatableClear(key);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        luaL_error(L, "key not found in datatable");
    }
    return 0;
}

int lua_datatableExport(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 0)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    // start operation
    QMetaObject::invokeMethod(g_mainWindow, [] {
        g_datatable->datatableExport();
    }, Qt::QueuedConnection);
    return 0;
}

int lua_dataplotAppend(lua_State *L) {
    // check arguments
    if (lua_gettop(L) > 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const int param2 = luaL_optinteger(L, 2, 0);
    // start operation
    const QString key = param1;
    const int position = param2;
    QMetaObject::invokeMethod(g_mainWindow, [key, position] {
        g_dataplot->dataplotAppend(key, position);
    }, Qt::QueuedConnection);
    return 0;
}