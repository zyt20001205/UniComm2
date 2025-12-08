#ifndef UNICOMM_LUAINTERPRETER_H
#define UNICOMM_LUAINTERPRETER_H

#include <QObject>
#include <QUrl>
#include <QMap>
#include <sol/state.hpp>

class LuaIO;
class LuaThread;

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QUrl &workspaceUrl, const QUrl &scriptUrl, const QVariantMap &luaSession , QObject *parent = nullptr);

    void start(const QString &script);

    void debugStateSet(int state) const;

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

    void showHeatmap() const;

    void hideHeatmap() const;

signals:
    void insertMarker(const QUrl &scriptUrl, int type, int line, int time);

    void removeMarker(const QUrl &scriptUrl, int type, int line);

    void appendLog(const QString &message, const QString &level);

    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);

private:
    static void luaRunHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    void handleError();

    sol::state m_lua{};
    QVariantMap m_luaSession{};
    LuaIO *m_luaIO{};
    LuaThread *m_luaThread{};
};

#endif //UNICOMM_LUAINTERPRETER_H
