#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <QObject>
#include <QSharedPointer>
#include <QUrl>
#include <lua.hpp>

struct DebugData {
    QUrl currentUrl;
    QString threadId;
    int depth = 0;
    int baseDepth = 0;
    int state;
};

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QUrl &rootUrl, const QUrl &scriptUrl, QObject *parent = nullptr);

    ~LuaInterpreter() override;

    void run(const QString &script) const;

    void debug(const QString &script, const DebugData &debugData);

    void debugStateSet(int state) const;

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

private:
    static void luaTerminateHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    void handleError() const;

    lua_State *L{};
    QUrl m_scriptUrl{};
    QSharedPointer<DebugData> m_debugData{};

    enum {
        DEBUG_RUN,
        DEBUG_PAUSE,
        DEBUG_STEPOVER,
        DEBUG_STEPINTO,
        DEBUG_STEPOUT
    };
};

#endif //LUAINTERPRETER_H
