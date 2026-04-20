#ifndef UNICOMM_LUAUTILS_H
#define UNICOMM_LUAUTILS_H

#include <QString>
#include <sol/sol.hpp>

void lua_pushvariant(lua_State *L, const QString &variant, const QString &type);

#endif //UNICOMM_LUAUTILS_H