#ifndef UNICOMM_LUAINTERPRETER_H
#define UNICOMM_LUAINTERPRETER_H

#include <QMap>
#include <QObject>
#include <QUrl>
#include <sol/state.hpp>

class LuaIO;
class LuaPort;
class LuaThread;

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QVariantMap &luaSession , QObject *parent = nullptr);

    void start(const QString &script);

    void stateSet(int state);

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

signals:
    void openScript(const QUrl &scriptUrl);

    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void appendLog(const QString &message, const QString &level);

    void listPort(std::vector<std::string> &portList);

    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);

    void quitLoop();

private:
    static void luaRunHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    sol::state m_lua{};
    QVariantMap m_luaSession{};
    LuaIO *m_luaIO{};
    LuaPort *m_luaPort{};
    LuaThread *m_luaThread{};
};

#endif //UNICOMM_LUAINTERPRETER_H
