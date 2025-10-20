#include "utils/luaUtils.h"

#include <QVariant>

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

void lua_pushqvariant(lua_State *L, const QVariant &value) {
    switch (value.typeId()) {
        case QMetaType::Bool: {
            lua_pushboolean(L, value.toBool());
            break;
        }
        case QMetaType::Int: {
            lua_pushinteger(L, value.toInt());
            break;
        }
        case QMetaType::QString: {
            lua_pushstring(L, value.toString().toUtf8().constData());
            break;
        }
        case QMetaType::QVariantList: {
            const QVariantList list = value.toList();
            lua_createtable(L, static_cast<int>(list.size()), 0);
            for (int i = 0; i < list.size(); ++i) {
                lua_pushqvariant(L, list[i]);
                lua_rawseti(L, -2, i + 1);
            }
            break;
        }
        case QMetaType::QVariantMap: {
            const QVariantMap map = value.toMap();
            lua_createtable(L, 0, static_cast<int>(map.size()));
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                lua_pushstring(L, it.key().toUtf8().constData());
                lua_pushqvariant(L, it.value());
                lua_settable(L, -3);
            }
            break;
        }
        default: {
            lua_pushnil(L);
            break;
        }
    }
}
