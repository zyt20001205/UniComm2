#ifndef LUAMISCELLANEOUS_H
#define LUAMISCELLANEOUS_H

#include <QApplication>
#include <QString>
#include <QThread>
#include <lua.hpp>
#include "script.h"
#include "log.h"

class Script;

extern Script *g_script;
extern Log *g_log;

QString lua_toqstring(lua_State *L, int idx);

void lua_pushqstring(lua_State *L, int idx, const QString &value);

int lua_exec(lua_State *L);

int lua_terminate(lua_State *L);

int lua_input(lua_State *L);

int lua_print(lua_State *L);

int lua_sleep(lua_State *L);

#endif //LUAMISCELLANEOUS_H
