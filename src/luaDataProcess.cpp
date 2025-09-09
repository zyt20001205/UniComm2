#include "../include/luaDataProcess.h"

int lua_databaseWrite(lua_State *L) {
    // check arguments
    if (lua_gettop(L) != 2)
        luaL_error(L, "unexpected number of arguments");
    // check arguments
    const char *param1 = luaL_checkstring(L, 1);
    const char *param2 = luaL_checkstring(L, 2);
    // start operation
    const QString key = param1;
    const QString value = param2;
    bool status;
    QMetaObject::invokeMethod(g_database, [key, value, &status] {
        status = g_database->databaseWrite(key, value);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        luaL_error(L, "key not found in database");
    }
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
    const QString key = param1;
    const QString value = param2;
    bool status;
    QMetaObject::invokeMethod(g_datatable, [key, value, &status] {
        status = g_datatable->datatableWrite(key, value);
    }, Qt::BlockingQueuedConnection);
    if (!status) {
        luaL_error(L, "key not found in datatable");
    }
    return 0;
}
