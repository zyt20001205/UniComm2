#include "luaModule/string.h"

String::String(QObject *parent)
    : QObject(parent) {
}

std::string String::toHex(const std::string_view &ba, const char separator) {
    const auto qba = QByteArray(ba.data(), ba.size()).toHex(separator).toUpper();
    return std::string(qba.constData(), qba.size());
}

std::string String::fromHex(const std::string &str) {
    const auto qba = QByteArray::fromHex(QByteArray::fromStdString(str));
    return std::string(qba.constData(), qba.size());
}
