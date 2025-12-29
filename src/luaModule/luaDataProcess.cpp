#include "luaModule/luaDataProcess.h"

#include "globals.h"
#include "utils/luaUtils.h"

LuaDataProcess::LuaDataProcess(QObject *parent)
    : QObject(parent) {
}

std::vector<std::string> LuaDataProcess::databaseList() {
    QSet<QString> databaseSet{};
    emit listDatabase(databaseSet);
    std::vector<std::string> databaseList{};
    for (const auto& key:databaseSet) {
        databaseList.push_back(key.toStdString());
    }
    return databaseList;
}

void LuaDataProcess::databaseWrite(const std::string &key, const std::string &value) {
    bool status = false;
    emit writeDatabase(QString::fromStdString(key), QString::fromStdString(value), status);
    if (!status) {
        throw sol::error("failed to write to database key: " + key);
    }
}

std::vector<std::string> LuaDataProcess::datatableList() {
    QSet<QString> datatableSet{};
    emit listDatatable(datatableSet);
    std::vector<std::string> datatableList{};
    for (const auto& key:datatableSet) {
        datatableList.push_back(key.toStdString());
    }
    return datatableList;
}

void LuaDataProcess::datatableWrite(const std::string &key, const std::string &value) {
    bool status = false;
    emit writeDatatable(QString::fromStdString(key), QString::fromStdString(value), status);
    if (!status) {
        throw sol::error("failed to write to datatable key: " + key);
    }
}

// int lua_dataplotAppend(lua_State *L) {
//     // check arguments
//     if (lua_gettop(L) > 2)
//         luaL_error(L, "unexpected number of arguments");
//     // convert arguments
//     const char *param1 = luaL_checkstring(L, 1);
//     const int param2 = luaL_optinteger(L, 2, 0);
//     // start operation
//     const QString key = param1;
//     const int position = param2;
//     QMetaObject::invokeMethod(g_mainWindow, [key, position] {
//         g_dataplot->dataplotAppend(key, position);
//     }, Qt::QueuedConnection);
//     return 0;
// }
//
// int lua_dataplotRemove(lua_State *L) {
//     // check arguments
//     if (lua_gettop(L) != 1)
//         luaL_error(L, "unexpected number of arguments");
//     // convert arguments
//     const char *param1 = luaL_checkstring(L, 1);
//     // start operation
//     const QString key = param1;
//     QMetaObject::invokeMethod(g_mainWindow, [key] {
//         g_dataplot->dataplotRemove(key);
//     }, Qt::QueuedConnection);
//     return 0;
// }

