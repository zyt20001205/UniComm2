#ifndef UNICOMM_LUAIO_H
#define UNICOMM_LUAIO_H

#include <QObject>

namespace sol {
    struct variadic_args;
}

class LuaIO final : public QObject {
    Q_OBJECT

public:
    explicit LuaIO(QObject *parent = nullptr);

    ~LuaIO() override = default;

    void log(const sol::variadic_args &args);

    // std::string inputDialog();

    void speak(const std::string &text);

signals:
    void appendLog(const QString &message, const QString &level);
};

#endif //UNICOMM_LUAIO_H
