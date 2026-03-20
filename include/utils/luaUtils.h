#ifndef UNICOMM_LUAUTILS_H
#define UNICOMM_LUAUTILS_H

#include <QString>
#include <lua.hpp>
#include "sol/object.hpp"

namespace sol {
    struct variadic_args;
}

QString SObject2QString(sol::object object);

QVariant SObject2QVariant(sol::object object, int depth = 0);

QVariantList SVariadicArgs2QVariantList(sol::variadic_args args);

QString lua2filepath(const std::string &luaPath);

void lua_pushvariant(lua_State *L, const QString &variant, const QString &type);

#endif //UNICOMM_LUAUTILS_H