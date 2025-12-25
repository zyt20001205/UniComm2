#ifndef UNICOMM_LUAIO_H
#define UNICOMM_LUAIO_H

#include <QHash>
#include <QObject>

class QEventLoop;

namespace sol {
    struct variadic_args;
}

class LuaIO final : public QObject {
    Q_OBJECT

public:
    explicit LuaIO(QObject *parent = nullptr);

    ~LuaIO() override = default;

    void log(const sol::variadic_args &args);

    void message(const std::string &text) const;

    void textField() const;

    static void speak(const std::string &text);

signals:
    void appendLog(const QString &message, const QString &level);

    void newMessageDialog(const QString &text, const QEventLoop *eventloop) const;
};

#endif //UNICOMM_LUAIO_H
