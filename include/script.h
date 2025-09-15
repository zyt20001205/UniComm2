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
#include <QShortcut>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSyntaxHighlighter>
#include <QTableWidget>
#include <QTextBrowser>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "luaDataProcess.h"
#include "luaPort.h"
#include "luaMiscellaneous.h"
#include "luaModbus.h"
#include "port.h"
#include "suffix.h"

// tab index
enum {
    DIAGNOSTICS_TAB,
    THREADPOOL_TAB,
    DEBUG_TAB,
};

// debug state
enum {
    STATE_RUN,
    STATE_PAUSE,
    STATE_TERMINATE,
    STATE_STEPOVER,
    STATE_STEPINTO,
    STATE_STEPOUT,
};

// editor marker/annotate
enum {
    MARKER_BREAKPOINT,
    MARKER_HIGHLIGHT,
};

enum {
    INDICATOR_ERROR,
    INDICATOR_WARNING,
    INDICATOR_HINT,
};

class Port;

class TooltipCompletion;

class TooltipHover;

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

    void scriptConfigSave() const;

    void scriptOpen(const QString &scriptUrl);

    void scriptHighlight(int row) const;

    void scriptTreeViewLoad(QStandardItemModel *varMap) const;

    void diagnosticsReturn(const QString &scriptUri, const QJsonArray &diagnosticsArray);

    void diagnosticsPublish() const;

    void completionReturn(const QJsonArray &items) const;

    void foldingRangeReturn(const QJsonArray &result) const;

    void formattingReturn(const QString &newText) const;

    void hoverReturn(const QString &message) const;

    void semanticTokensReturn(const QJsonArray &data) const;

signals:
    void appendLog(const QString &message, const QString &level);

    void debugResume();

    void showManual(const QString &func);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void scriptRun();

    void scriptRunning(const QString &name, QThread *worker);

    void scriptDebug();

    void scriptModify(int index) const;

    void scriptClose(int index);

    void scriptSelected(int index);

    void scriptSwap(int srcIndex, int dstIndex);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();
    QTabWidget *m_scriptTabWidget = nullptr;
    ScriptPageWidget *m_currentScriptPage = nullptr;
    QHash<QString, QJsonArray> m_diagnosticsHash = {};
    TooltipHover *m_tooltipHover = nullptr;
    QTabWidget *m_scriptMonitorTabWidget = nullptr;
    QTableWidget *m_scriptDiagnosticsTableWidget = nullptr;
    QListWidget *m_scriptThreadPoolListWidget = nullptr;
    LuaInterpreter *m_debugInterpreter = nullptr;
    QWidget *m_scriptDebugWidget = nullptr;
    QTreeView *m_scriptDebugTreeView = nullptr;
    ScriptExplorer *m_scriptExplorerTreeView = nullptr;

    enum {
        TOKENTYPE_NAMESPACE,
        TOKENTYPE_TYPE,
        TOKENTYPE_CLASS,
        TOKENTYPE_ENUM,
        TOKENTYPE_INTERFACE,
        TOKENTYPE_STRUCT,
        TOKENTYPE_TYPEPARAMETER,
        TOKENTYPE_PARAMETER,
        TOKENTYPE_VARIABLE,
        TOKENTYPE_PROPERTY,
        TOKENTYPE_ENUMMEMBAER,
        TOKENTYPE_EVENT,
        TOKENTYPE_FUNCTION,
        TOKENTYPE_METHOD,
        TOKENTYPE_MACRO,
        TOKENTYPE_KEYWORD,
        TOKENTYPE_MODIFIER,
        TOKENTYPE_COMMENT,
        TOKENTYPE_STRING,
        TOKENTYPE_NUMBER,
        TOKENTYPE_REGEXP,
        TOKENTYPE_OPERATOR,
        TOKENTYOE_DECORATOR,
    };

    enum {
        TOKENMODIFIERS_DECLARATION = 1 << 0,
        TOKENMODIFIERS_DEFINITION = 1 << 1,
        TOKENMODIFIERS_READONLY = 1 << 2,
        TOKENMODIFIERS_STATIC = 1 << 3,
        TOKENMODIFIERS_DEPRECATED = 1 << 4,
        TOKENMODIFIERS_ABSTRACT = 1 << 5,
        TOKENMODIFIERS_ASYNC = 1 << 6,
        TOKENMODIFIERS_MODIFICATION = 1 << 7,
        TOKENMODIFIERS_DOCUMENTATION = 1 << 8,
        TOKENMODIFIERS_DEFAULTLIBRARY = 1 << 9,
        TOKENMODIFIERS_GLOBAL = 1 << 10,
    };

    enum {
        LUATOKEN_PARAMETER = 64,
        LUATOKEN_VARIABLE,
        LUATOKEN_FUNCTION_DECLARATION,
        LUATOKEN_FUNCTION_CALL,
        LUATOKEN_METHOD,
        LUATOKEN_KEYWORD,
        LUATOKEN_COMMENT,
        LUATOKEN_STRING,
        LUATOKEN_NUMBER,
        LUATOKEN_OPERATOR,
    };
};

class TooltipCompletion final : public QWidget {
    Q_OBJECT

public:
    explicit TooltipCompletion(QWidget *parent = nullptr);

    ~TooltipCompletion() override = default;

    void showTooltip(const QJsonArray &items);

    void hideTooltip();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTableWidget *m_tableWidget = nullptr;
};

class TooltipHover final : public QWidget {
    Q_OBJECT

public:
    explicit TooltipHover(QWidget *parent = nullptr);

    ~TooltipHover() override = default;

    void showTooltip(const QString &message);

    void hideTooltip();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTextBrowser *m_textBrowser = nullptr;
};

class ScriptPageWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ScriptPageWidget(const QJsonObject &scriptConfig = QJsonObject(), const QString &scriptUrl = QString(), QWidget *parent = nullptr);

    ~ScriptPageWidget() override = default;

    void scriptSave();

    void scriptEditFinish();

    void completionRequest();

    void foldingRangeRequest();

    void formattingRequest();

    void semanticTokensRequest();

    int m_version = 1;
    ScriptEditor *m_scriptEditor = nullptr;
    QString m_scriptUrl;
    bool m_scriptModify = false;
    TooltipCompletion *m_tooltipCompletion = nullptr;

signals:
    void modifyScript();

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private slots:
    void scriptModify(bool status);

    void scriptEdit() const;

    void dwellStart(int pos, int x, int y);

private:
    void didChangeNotification();

    void didOpenNotification();

    void hoverRequest(int line, int character);

    QTimer *m_editTimer = nullptr;
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

private slots:
    void onMarginClick(int margin, int line, Qt::KeyboardModifiers state);

private:
    void breakpointUpdate() const;
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
