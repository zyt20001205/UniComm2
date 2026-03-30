#ifndef UNICOMM_LUAKEY_H
#define UNICOMM_LUAKEY_H

#include <QHash>
#include <QObject>

class LuaKey final : public QObject {
    Q_OBJECT

public:
    explicit LuaKey(QObject *parent = nullptr);

    ~LuaKey() override = default;

    void tap(const std::string &key);

    static void type(const std::string &text);

private:
    QHash<QString, int> m_vkHash{};
};

#endif //UNICOMM_LUAKEY_H