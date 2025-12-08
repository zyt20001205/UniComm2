#include "luaModule/luaIO.h"

#include <QVariant>
#include <sol/object.hpp>
#include <sol/variadic_args.hpp>

#include "utils/luaUtils.h"

LuaIO::LuaIO(QObject *parent)
    : QObject(parent) {
}

void LuaIO::log(const sol::variadic_args &args) {
    QVariantList parsedList = lua2qt(args);
    for (const auto &parsed: parsedList) {
        emit appendLog(parsed.toString(), "info");
    }
}
