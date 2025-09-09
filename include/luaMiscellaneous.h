#ifndef LUAMISCELLANEOUS_H
#define LUAMISCELLANEOUS_H

#include <QApplication>
#include <QString>
#include <QThread>
#include <lua.hpp>
#include "log.h"

extern Log *g_log;

QString lua_toqstring(lua_State *L, int idx);

void lua_pushqstring(lua_State *L, int idx, const QString &value);

int lua_input(lua_State *L);

int lua_print(lua_State *L);

int lua_sleep(lua_State *L);

#endif //LUAMISCELLANEOUS_H
