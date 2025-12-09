#ifndef UNICOMM_LUATHREAD_H
#define UNICOMM_LUATHREAD_H

#include <QObject>

class LuaThread final : public QObject {
    Q_OBJECT

public:
    explicit LuaThread(QObject *parent = nullptr);

    ~LuaThread() override = default;

    std::string start(const std::string &scriptPath);

    void stop(const std::string &threadId);

    void sleep(int ms);

    void join(const std::string &threadId);

signals:
    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);

    void joinThread(const QString &threadId);
};

#endif //UNICOMM_LUATHREAD_H
