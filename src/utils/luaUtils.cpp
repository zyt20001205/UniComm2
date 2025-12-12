#include "utils/luaUtils.h"

#include <QVariant>
#include <sol/object.hpp>
#include "sol/table_core.hpp"
#include <sol/variadic_args.hpp>

QVariant lua2qvar(sol::object object, int depth) {
    constexpr int MAX_DEPTH = 100;
    if (depth > MAX_DEPTH) {
        throw sol::error("Maximum recursion depth exceeded");
    }
    QVariant parsed{};
    switch (object.get_type()) {
        case sol::type::nil: {
            parsed = "nil";
        }
        break;
        case sol::type::string: {
            const std::string str = object.as<std::string>();
            bool raw = false;
            for (const char ch: str) {
                if ((ch >= 0x00 && ch <= 0x1F) || ch == 0x7F) {
                    raw = true;
                    break;
                }
            }
            if (raw) {
                QByteArray byteArray(str.data(), static_cast<qsizetype>(str.size()));
                parsed = byteArray.toHex(' ').toUpper();
                break;
            }
            QString string{};
            // try utf-8
            string = QString::fromUtf8(str.data(), static_cast<qsizetype>(str.size()));
            if (!string.contains(QChar::ReplacementCharacter)) {
                parsed = string;
                break;
            }
            // try ascii
            string = QString::fromLatin1(str.data(), static_cast<qsizetype>(str.size()));
            parsed = string;
        }
        break;
        case sol::type::number: {
            if (object.is<int>()) {
                parsed = object.as<int>();
            } else if (object.is<double>()) {
                parsed = object.as<double>();
            } else {
                qDebug() << "Unsupported Number Type";
                parsed = "?";
            }
        }
        break;
        case sol::type::boolean: {
            parsed = object.as<bool>();
        }
        break;
        case sol::type::table: {
            const auto table = object.as<sol::table>();
            QVariantMap map;
            for (const auto &[key, value]: table) {
                QString key_str;
                if (key.is<std::string>()) {
                    key_str = QString::fromStdString(key.as<std::string>());
                } else if (key.is<int>()) {
                    key_str = QString::number(key.as<int>());
                } else if (key.is<double>()) {
                    key_str = QString::number(key.as<double>());
                } else {
                    continue;
                }
                map[key_str] = lua2qvar(value, depth + 1);
            }
            parsed = QVariant::fromValue(map);
        }
        break;
        default: {
            qDebug() << "Unsupported Lua Type" << static_cast<int>(object.get_type());;
            parsed = "?";
        }
        break;
    }
    return parsed;
}

QVariantList lua2qvarlist(sol::variadic_args args) {
    QVariantList parsedList{};
    for (const sol::object arg: args) {
        QVariant parsed = lua2qvar(arg);
        parsedList.append(parsed);
    }
    return parsedList;
}

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
