#include "util/luaUtils.h"

#include <QDir>
#include <QVariant>

void lua_pushvariant(lua_State *L, const QString &variant, const QString &type) {
    if (type == "boolean") {
        const QVariant tmp(variant);
        const auto value = tmp.toBool();
        lua_pushboolean(L, value);
    } else if (type == "number") {
        bool ok = false;
        // try integer
        const auto integerValue = variant.toInt(&ok);
        if (ok) {
            lua_pushinteger(L, integerValue);
        }
        // parse as double
        const auto doubleValue = variant.toDouble();
        lua_pushnumber(L, doubleValue);
    } else if (type == "string") {
        lua_pushstring(L, variant.toUtf8().constData());
    }
}
