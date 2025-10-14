#include "utils/luaUtils.h"

QString lua_toqstring(lua_State *L, const int idx) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            return QString("\\");
        case LUA_TBOOLEAN:
            return lua_toboolean(L, idx) ? "true" : "false";
        case LUA_TLIGHTUSERDATA:
            return QString("WIP");
        case LUA_TNUMBER:
            return lua_tostring(L, idx);
        case LUA_TSTRING:
            return lua_tostring(L, idx);
        case LUA_TTABLE:
            return QString("{...}");
        case LUA_TFUNCTION:
            return QString("\\");
        case LUA_TUSERDATA:
            return QString("WIP");
        case LUA_TTHREAD:
            return QString("\\");
        default:
            return QString("?");
    }
}

void lua_pushqstring(lua_State *L, const int idx, const QString &value) {
    switch (lua_type(L, idx)) {
        case LUA_TBOOLEAN: {
            if (value == "true" || value == "1") {
                lua_pushboolean(L, 1);
            } else {
                lua_pushboolean(L, 0);
            }
        }
            break;
        case LUA_TNUMBER: {
            if (value.contains(".") || value.contains("e")) {
                lua_pushnumber(L, value.toDouble());
            } else {
                lua_pushinteger(L, value.toInt());
            }
        }
            break;
        case LUA_TSTRING: {
            lua_pushstring(L, value.toUtf8().constData());
        }
            break;
        default: break;
    }
}