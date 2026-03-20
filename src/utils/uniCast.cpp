#include "utils/uniCast.h"

#include <sol/table_core.hpp>
#include <sol/variadic_args.hpp>
#include <sol/userdata.hpp>

// sol -> qt
template<>
QString uni_cast<sol::object, QString>(const sol::object &s, const int depth) {
    switch (s.get_type()) {
        case sol::type::nil:
            return "nil";
        case sol::type::boolean:
            return s.as<bool>() ? "true" : "false";
        case sol::type::lightuserdata:
            return "lightuserdata";
        case sol::type::number:
            if (s.is<int>()) {
                return QString::number(s.as<int>());
            }
            return QString::number(s.as<double>());
        case sol::type::string:
            return QString::fromStdString(s.as<std::string>());
        case sol::type::table:
            return "{...}";
        case sol::type::function:
            return "function";
        case sol::type::userdata:
            return "userdata";
        case sol::type::thread:
            return "thread";
        default:
            return "?";
    }
}

template<>
QVariant uni_cast<sol::object, QVariant>(const sol::object &s, const int depth) {
    constexpr int MAX_DEPTH = 100;
    if (depth > MAX_DEPTH) {
        throw sol::error("Maximum recursion depth exceeded");
    }
    switch (s.get_type()) {
        case sol::type::nil: {
            return "nil";
        }
        case sol::type::boolean: {
            return s.as<bool>();
        }
        case sol::type::number: {
            if (s.is<int>()) {
                return s.as<int>();
            }
            return s.as<double>();
        }
        case sol::type::string: {
            const std::string str = s.as<std::string>();
            bool raw = false;
            for (const char ch: str) {
                if ((ch >= 0x00 && ch <= 0x1F) || ch == 0x7F) {
                    raw = true;
                    break;
                }
            }
            if (raw) {
                const QByteArray byteArray(str.data(), static_cast<qsizetype>(str.size()));
                return byteArray.toHex(' ').toUpper();
            }
            QString string{};
            // try utf-8
            string = QString::fromUtf8(str.data(), static_cast<qsizetype>(str.size()));
            if (!string.contains(QChar::ReplacementCharacter)) {
                return string;
            }
            // try ascii
            string = QString::fromLatin1(str.data(), static_cast<qsizetype>(str.size()));
            return string;
        }
        case sol::type::table: {
            const auto table = s.as<sol::table>();
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
                map[key_str] = uni_cast<sol::object, QVariant>(value, depth + 1);
            }
            return QVariant::fromValue(map);
        }
        case sol::type::userdata: {
            const auto mapPtr = s.as<sol::userdata>().as<QVariantMap *>();
            return QVariant::fromValue(*mapPtr);
        }
        default: {
            qDebug() << "Unsupported Lua Type" << static_cast<int>(s.get_type());;
            return "?";
        }
    }
}

template<>
QVariantList uni_cast<sol::variadic_args, QVariantList>(const sol::variadic_args &s, const int depth) {
    QVariantList d{};
    for (const sol::object &arg: s) {
        QVariant parsed = uni_cast<sol::object, QVariant>(arg);
        d.append(parsed);
    }
    return d;
}

// qt -> std
template<>
std::vector<std::string> uni_cast<QSet<QString>, std::vector<std::string>>(const QSet<QString> &s, int depth) {
    std::vector<std::string> d{};
    for (const auto &value: s) {
        d.push_back(value.toStdString());
    }
    return d;
}