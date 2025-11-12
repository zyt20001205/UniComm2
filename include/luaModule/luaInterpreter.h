#ifndef UNICOMM_LUAINTERPRETER_H
#define UNICOMM_LUAINTERPRETER_H

#include <QHash>
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
    QHash<QUrl, QList<int>> heatmap;
};

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QUrl &workspaceUrl, const QUrl &scriptUrl, QObject *parent = nullptr);

    ~LuaInterpreter() override;

    void run(const QString &script) const;

    void debug(const QString &script, const DebugData &debugData);

    void debugStateSet(int state) const;

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

    void showHeatmap() const;

    void hideHeatmap() const;

private:
    static void luaTerminateHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    void handleError() const;

    lua_State *L{};
    QUrl m_scriptUrl{};
    QSharedPointer<DebugData> m_debugData{};
};

#endif //UNICOMM_LUAINTERPRETER_H
