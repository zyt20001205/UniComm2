#ifndef LUAMISCELLANEOUS_H
#define LUAMISCELLANEOUS_H

#include <QString>
#include <lua.hpp>

QString lua_toqstring(lua_State* L, int idx);

void lua_pushqstring(lua_State* L, int idx, const QString& value);

#endif //LUAMISCELLANEOUS_H