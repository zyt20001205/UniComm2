#ifndef UNICOMM_LUASTRING_H
#define UNICOMM_LUASTRING_H

#include <QObject>

class LuaString final : public QObject {
    Q_OBJECT

public:
    explicit LuaString(QObject *parent = nullptr);

    ~LuaString() override = default;

    static std::string toHex(const std::string_view &ba, char separator);

    static std::string fromHex(const std::string &str);
};

#endif //UNICOMM_LUASTRING_H