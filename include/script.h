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
#include <QSyntaxHighlighter>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "port.h"

class Port;

class ScriptPageWidget;

class LuaLexer;

class ScriptEditor;

class ScriptExplorer;

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

    void scriptConfigSave() const;

    void scriptOpen(const QString &scriptPath);

signals:
    void appendLog(const QString &message, const QString &level);

    void writeDatabase(const QString &key, const QString &value);

    void writeDatatable(const QString &key, const QString &value);

private:
    void scriptRun();

    void scriptRunning(const QString &name, QThread *worker);

    void scriptEdited(int index) const;

    void scriptClose(int index) const;

    static int luaPrint(lua_State *L);

    static int luaSleep(lua_State *L);

    static int luaInput(lua_State *L);

    static int luaPortOpen(lua_State *L);

    static int luaPortClose(lua_State *L);

    static int luaPortInfo(lua_State *L);

    static int luaPortWriteText(lua_State *L);

    static int luaPortWriteData(lua_State *L);

    static int luaPortReadText(lua_State *L);

    static int luaPortReadData(lua_State *L);

    static int luaDatabaseWrite(lua_State *L);

    static int luaDatatableWrite(lua_State *L);

    // static int luaModbusRtuReadHoldingRegisters(lua_State *L);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();
    Port *m_port = nullptr;
    QTabWidget *m_scriptTabWidget = nullptr;
    QListWidget *m_scriptListWidget = nullptr;
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
};

class ScriptEditor final : public QsciScintilla {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;

    LuaLexer *m_scriptLexer = nullptr;
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
