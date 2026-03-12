#include "luaModule/luaString.h"

LuaString::LuaString(QObject *parent)
    : QObject(parent) {
}

std::string LuaString::toHex(const std::string_view &ba, const char separator) {
    const auto qba = QByteArray(ba.data(), ba.size()).toHex(separator);
    return std::string(qba.constData(), qba.size());
}

std::string LuaString::fromHex(const std::string &str) {
    const auto qba = QByteArray::fromHex(QByteArray::fromStdString(str));
    return std::string(qba.constData(), qba.size());
}
