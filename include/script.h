#ifndef SCRIPT_H
#define SCRIPT_H

#include <QDockWidget>
#include <QDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <Qsci/qsciapis.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qsciscintilla.h>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "luaMiscellaneous.h"
#include "port.h"
#include "suffix.h"

class Port;

class ScriptPageWidget;

class LuaLexer;

class ScriptEditor;

class LuaInterpreter;

class ScriptExplorer;

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

    void scriptConfigSave() const;

    void scriptOpen(const QString &scriptPath);

    void scriptHighlight(int row) const;

    void scriptTreeViewLoad(QStandardItemModel *varMap) const;

    Port *m_port = nullptr;
signals:
    void appendLog(const QString &message, const QString &level);

    void debugResume();

    void showManual(const QString &func);

    void writeDatabase(const QString &key, const QString &value);

    void writeDatatable(const QString &key, const QString &value);

private:
    void scriptRun();

    void scriptRunning(const QString &name, QThread *worker);

    void scriptDebug();

    void scriptEdited(int index) const;

    void scriptClose(int index);

    void scriptSwap(int srcIndex, int dstIndex);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();
    QTabWidget *m_scriptTabWidget = nullptr;
    QTabWidget *m_scriptMonitorTabWidget = nullptr;
    QListWidget *m_scriptThreadPoolListWidget = nullptr;
    LuaInterpreter *m_debugInterpreter = nullptr;
    QWidget *m_scriptDebugWidget = nullptr;
    QTreeView *m_scriptDebugTreeView = nullptr;
    ScriptExplorer *m_scriptExplorerTreeView = nullptr;
};

class ScriptPageWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ScriptPageWidget(const QJsonObject &scriptConfig = QJsonObject(), const QString &scriptPath = QString(), QObject *parent = nullptr);

    ~ScriptPageWidget() override = default;

    void scriptSave();

    ScriptEditor *m_scriptEditor = nullptr;
    QString m_scriptPath;
    bool m_scriptEdited = false;

signals:
    void showManual(const QString &func);

    void editScript();

private slots:
    void scriptEdited();
};

class LuaLexer final : public QsciLexerLua {
    Q_OBJECT

public:
    using QsciLexerLua::QsciLexerLua;

    const char *wordCharacters() const override {
        return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.";
    }

    QStringList autoCompletionWordSeparators() const override {
        return {};
    }
};

class ScriptEditor final : public QsciScintilla {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;

    LuaLexer *m_scriptLexer = nullptr;

signals:
    void showManual(const QString &func);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void breakpointUpdate() const;

private slots:
    void onMarginClicked(int margin, int line, Qt::KeyboardModifiers state);
};

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(QObject *parent = nullptr);

    ~LuaInterpreter() override = default;

    void run(const QString &script) const;

    void debug(const QString &script);

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

private:
    static void luaTerminateHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    static int luaPortOpen(lua_State *L);

    static int luaPortClose(lua_State *L);

    static int luaPortInfo(lua_State *L);

    static int luaPortWriteText(lua_State *L);

    static int luaPortWriteData(lua_State *L);

    static int luaPortReadText(lua_State *L);

    static int luaPortReadData(lua_State *L);

    static int luaModbusRtuReadHoldingRegisters(lua_State *L);

    static int luaModbusRtuWriteMultipleRegisters(lua_State *L);

    static int luaModbusAsciiReadHoldingRegisters(lua_State *L);

    static int luaDatabaseWrite(lua_State *L);

    static int luaDatatableWrite(lua_State *L);

    lua_State *L = nullptr;
    lua_State *co = nullptr;
};

class ScriptExplorer final : public QTreeView {
    Q_OBJECT

public:
    explicit ScriptExplorer(QWidget *parent = nullptr);

    ~ScriptExplorer() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QString &scriptPath);

    void runScript(const QString &name, const QString &script);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void scriptRun(const QModelIndex &index);

    void scriptOpen(const QModelIndex &index);

    void scriptDelete(const QModelIndex &index);

    void scriptNew();

    static void scriptOpenInExplorer();

    QFileSystemModel *m_model = nullptr;
};

#endif //SCRIPT_H
