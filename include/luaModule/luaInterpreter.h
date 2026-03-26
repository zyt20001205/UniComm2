#ifndef UNICOMM_LUAINTERPRETER_H
#define UNICOMM_LUAINTERPRETER_H

#include <QMap>
#include <QObject>
#include <QUrl>
#include <sol/state.hpp>

class QEventLoop;
class QStandardItemModel;
class LuaDataProcess;
class LuaIO;
class LuaModbusAscii;
class LuaModbusRtu;
class LuaPort;
class LuaSmtp;
class LuaString;
class LuaThread;

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QVariantMap &luaSession , QObject *parent = nullptr);

    void start(const QString &script);

    void stateSet(int state);

    static void stackSet(lua_State *L, lua_Debug *ar);

    static void watchSet(lua_State *L, lua_Debug *ar);

    void valueSet(const QString &scriptUrl, const QString &expression, const QString &value, const QString &type);

signals:
    void openScript(const QUrl &scriptUrl);

    void addMarker(const QUrl &scriptUrl, int type, int line, int time);

    void deleteMarker(const QUrl &scriptUrl, int type, int line);

    void insertCallStack(const QString &threadId, QStandardItemModel *callStackModel);

    void appendLog(const QString &message, const QString &level);

    void newMessageDialog(const QEventLoop *eventloop, const QString &threadId, const QString &text) const;

    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);

    void setValue(const QString &scriptUrl, const QString &expression, const QString &value, const QString &type);

    void quitLoop();

private:
    static void luaRunHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    sol::state m_lua{};
    QVariantMap m_luaSession{};
    LuaDataProcess *m_luaDataProcess{};
    LuaIO *m_luaIO{};
    LuaModbusAscii *m_luaModbusAscii{};
    LuaModbusRtu *m_luaModbusRtu{};
    LuaPort *m_luaPort{};
    LuaSmtp *m_luaSmtp{};
    LuaString *m_luaString{};
    LuaThread *m_luaThread{};
};

#endif //UNICOMM_LUAINTERPRETER_H
