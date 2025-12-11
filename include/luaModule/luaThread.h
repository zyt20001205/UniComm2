#ifndef UNICOMM_LUATHREAD_H
#define UNICOMM_LUATHREAD_H

#include <QObject>

namespace sol {
    struct this_state;
}

class LuaThread final : public QObject {
    Q_OBJECT

public:
    explicit LuaThread(QObject *parent = nullptr);

    ~LuaThread() override = default;

    std::string start(sol::this_state ts, const std::string &scriptPath);

    void stop(const std::string &threadId);

    void sleep(int ms);

signals:
    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);
};

#endif //UNICOMM_LUATHREAD_H
