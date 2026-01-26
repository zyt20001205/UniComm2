#ifndef UNICOMM_LUAUTILS_H
#define UNICOMM_LUAUTILS_H

#include <QString>
#include <lua.hpp>
#include "sol/object.hpp"

namespace sol {
    struct variadic_args;
}

QVariant lua2qvar(sol::object object, int depth = 0);

QVariantList lua2qvarlist(sol::variadic_args args);

QString lua2qstring(sol::object object);

QString lua2filepath(const std::string &luaPath);

void lua_pushvariant(lua_State *L, const QString &variant, const QString &type);

// void lua_pushqvariant(lua_State *L, const QVariant &value);

#endif //UNICOMM_LUAUTILS_H