#include "api/string.h"

String::String(QObject *parent)
    : QObject(parent) {
}

std::string String::toBase64(const std::string &str) {
    const auto qba = QByteArray::fromStdString(str).toBase64();
    return std::string(qba.data(), qba.size());
}

std::string String::fromBase64(const std::string &str) {
    const auto qba = QByteArray::fromBase64(QByteArray::fromStdString(str));
    return std::string(qba.data(), qba.size());
}

std::string String::toHex(const std::string &str, const char separator) {
    const auto qba = QByteArray::fromStdString(str).toHex(separator).toUpper();
    return std::string(qba.data(), qba.size());
}

std::string String::fromHex(const std::string &str) {
    const auto qba = QByteArray::fromHex(QByteArray::fromStdString(str));
    return std::string(qba.data(), qba.size());
}
