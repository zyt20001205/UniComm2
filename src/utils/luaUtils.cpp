#include "utils/luaUtils.h"

#include <QDir>
#include <QVariant>
#include <sol/object.hpp>
#include <sol/table_core.hpp>
#include <sol/variadic_args.hpp>
#include <sol/userdata.hpp>

#include "globals.h"

QString SObject2QString(sol::object object) {
    switch (object.get_type()) {
        case sol::type::nil:
            return {"nil"};
        case sol::type::boolean:
            return object.as<bool>() ? "true" : "false";
        case sol::type::lightuserdata:
            return {"lightuserdata"};
        case sol::type::number:
            if (object.is<int>()) {
                return QString::number(object.as<int>());
            } else {
                return QString::number(object.as<double>());
            }
        case sol::type::string:
            return QString::fromStdString(object.as<std::string>());
        case sol::type::table:
            return {"{...}"};
        case sol::type::function:
            return {"function"};
        case sol::type::userdata:
            return {"userdata"};
        case sol::type::thread:
            return {"thread"};
        default:
            return {"?"};
    }
}

QVariant SObject2QVariant(sol::object object, int depth) {
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
        case sol::type::boolean: {
            parsed = object.as<bool>();
        }
        break;
        case sol::type::number: {
            if (object.is<int>()) {
                parsed = object.as<int>();
            } else {
                parsed = object.as<double>();
            }
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
                map[key_str] = SObject2QVariant(value, depth + 1);
            }
            parsed = QVariant::fromValue(map);
        }
        break;
        case sol::type::userdata: {
            auto mapPtr = object.as<sol::userdata>().as<QVariantMap *>();
            parsed = QVariant::fromValue(*mapPtr);
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

QVariantList SVariadicArgs2QVariantList(sol::variadic_args args) {
    QVariantList parsedList{};
    for (const sol::object arg: args) {
        QVariant parsed = SObject2QVariant(arg);
        parsedList.append(parsed);
    }
    return parsedList;
}

QString lua2filepath(const std::string &luaPath) {
    const QString relativePath = QString::fromStdString(luaPath);
    const QDir workspaceDir(g_workspaceUrl.toLocalFile());
    const QString filePath = workspaceDir.absoluteFilePath(relativePath);
    return filePath;
}

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
