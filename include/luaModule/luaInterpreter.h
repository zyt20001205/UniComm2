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
class LuaModbusRtu;
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

    void insertCallStack(const QString &threadId, QStandardItemModel *callStackModel);

    void listDatabase(QSet<QString> &databaseSet);

    void writeDatabase(const QString &key, const QString &value, bool &status);

    void listDatatable(QSet<QString> &datatableSet);

    void writeDatatable(const QString &key, const QString &value, bool &status);

    void appendLog(const QString &message, const QString &level);

    void newMessageDialog(const QString &threadId, const QString &text, const QEventLoop *eventloop) const;

    void listPort(QSet<QString> &portSet);

    void startThread(const QString &scriptPath, int mode, QString &threadId);

    void stopThread(const QString &threadId);

    void quitLoop();

private:
    static void luaRunHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    sol::state m_lua{};
    QVariantMap m_luaSession{};
    LuaDataProcess *m_luaDataProcess{};
    LuaIO *m_luaIO{};
    LuaModbusRtu *m_luaModbusRtu{};
    LuaPort *m_luaPort{};
    LuaThread *m_luaThread{};
};

#endif //UNICOMM_LUAINTERPRETER_H
