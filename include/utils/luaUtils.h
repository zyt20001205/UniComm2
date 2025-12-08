#ifndef UNICOMM_LUAUTILS_H
#define UNICOMM_LUAUTILS_H

#include <QString>
#include <lua.hpp>

namespace sol {
    struct variadic_args;
}

QVariantList lua2qt(sol::variadic_args args);

QString lua_toqstring(lua_State *L, int idx);

void lua_pushqstring(lua_State *L, int idx, const QString &value);

void lua_pushqvariant(lua_State *L, const QVariant &value);

#endif //UNICOMM_LUAUTILS_H