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
#include <QPointer>
#include <lua.hpp>
#include "config.h"
#include "luaControl.h"
#include "luaDataProcess.h"
#include "luaPort.h"
#include "luaMiscellaneous.h"
#include "luaModbus.h"
#include "port.h"
#include "suffix.h"
#include "utils.h"

// debug state
enum {
    STATE_RUN,
    STATE_PAUSE,
    STATE_TERMINATE,
    STATE_STEPOVER,
    STATE_STEPINTO,
    STATE_STEPOUT,
};

struct DebugData {
    QUrl currentUrl;
    QHash<QUrl, QSet<int> > *breakpoints;
};

// editor marker/annotate
enum {
    MARKER_BREAKPOINT,
    MARKER_HIGHLIGHT,
};

enum {
    INDICATOR_ERROR = 1,
    INDICATOR_WARNING,
    INDICATOR_INFO,
    INDICATOR_HINT,
    INDICATOR_HIGHLIGHT,
};

class Port;

class ScriptPage;

class ScriptEditor;

class LuaInterpreter;

class ScriptExplorer;

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void workspaceOpen(const QUrl &rootUrl);

    void scriptConfigSave();

    void scriptOpen(const QUrl &scriptUrl);

    void scriptExec(const QString &scriptPath);

    void cursorPositionSet(const QUrl &scriptUrl, int startLine, int startCharacter);

    void annotateHighlight(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void markerHighlight(int row) const;

    void scriptTreeViewLoad(QStandardItemModel *varMap) const;

    void diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void completionReturn(const QUrl &scriptUrl,const QJsonArray &items) const;

    void foldingRangeReturn(const QUrl &scriptUrl,const QJsonArray &result) const;

    void formattingReturn(const QUrl &scriptUrl,const QString &newText) const;

    void hoverReturn(const QUrl &scriptUrl,const QString &message) const;

    void semanticTokensReturn(const QUrl &scriptUrl, const QJsonArray &data) const;

    void signatureHelpReturn(const QUrl &scriptUrl,const QJsonObject &signature) const;

    ScriptExplorer *m_scriptExplorerTreeView = nullptr;
signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace(const QUrl &rootUrl);

    void debugResume();

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void scriptRun(const QString &script);

    void scriptRunning(const QString &name, QThread *worker);

    void scriptDebug();

    void scriptModify(int index) const;

    void scriptClose(int index);

    void scriptSwap(int srcIndex, int dstIndex);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();
    QUrl m_rootUrl{};
    QHash<QUrl, QSet<int> > m_breakpoints;
    QHash<QUrl, QJsonArray> m_diagnosticsHash{};
    QTabWidget *m_scriptTabWidget = nullptr;
    QList<QUrl> m_scriptList{};
    QHash<QUrl, ScriptPage *> m_scriptPageHash{};
    ScriptPage *m_currentScriptWidget = nullptr;
    QTabWidget *m_scriptMonitorTabWidget = nullptr;
    QListWidget *m_scriptThreadPoolListWidget = nullptr;
    LuaInterpreter *m_debugInterpreter = nullptr;
    QWidget *m_scriptDebugWidget = nullptr;
    QTreeView *m_scriptDebugTreeView = nullptr;

    // ui related
    enum {
        THREADPOOL_TAB,
        DEBUG_TAB,
    };
};

class TooltipCompletion;

class TooltipHover;

class TooltipPosition;

class TooltipSignatureHelp;

class ScriptPage final : public QWidget {
    Q_OBJECT

public:
    explicit ScriptPage(const QJsonObject &scriptConfig = QJsonObject(), const QUrl &scriptUrl = QUrl(), QWidget *parent = nullptr);

    ~ScriptPage() override = default;

    void scriptSave() const;

    void completionReturn(const QJsonArray &items) const;

    void diagnosticsReturn(const QJsonArray &diagnosticsArray) const;

    void foldingRangeReturn(const QJsonArray &result) const;

    void formattingReturn(const QString &newText) const;

    void hoverReturn(const QString &message) const;

    void semanticTokensReturn(const QJsonArray &data) const;

    void signatureHelpReturn(const QJsonObject &signature) const;

    ScriptEditor *m_scriptEditor = nullptr;
    QUrl m_scriptUrl;
    bool m_scriptModify = false;
    TooltipCompletion *m_tooltipCompletion = nullptr;
    TooltipHover *m_tooltipHover = nullptr;
    TooltipPosition *m_tooltipPosition = nullptr;
    TooltipSignatureHelp *m_tooltipSignatureHelp = nullptr;

signals:
    void modifyScript();

    void insertBreakpoint(const QUrl &scriptUrl, int line);

    void removeBreakpoint(const QUrl &scriptUrl, int line);

    void requestJson(const QString &method, const QJsonObject &params);

    void notificationJson(const QString &method, const QJsonObject &params);

private slots:
    void scriptModify(bool status);

    void scriptEdit() const;

    void dwellStart(int pos, int x, int y);

    void marginClick(int margin, int line, Qt::KeyboardModifiers state);

private:
    void scriptEditFinish();

    void completionRequest();

    void didChangeNotification();

    void didOpenNotification();

    void foldingRangeRequest();

    void formattingRequest();

    void semanticTokensRequest();

    void signatureHelpRequest();

    void dwellSwitch(bool status) const;

    void hoverRequest(int line, int character);

    void textReplace(QString &text, const QString &kind) const;

    void textInsert(QString &text, const QString &kind) const;

    void positionFill(int x, int y) const;

    int m_version = 1;
    QTimer *m_editTimer = nullptr;

    // semantic related
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
        LUATOKEN_TYPE = 64,
        LUATOKEN_PARAMETER,
        LUATOKEN_VARIABLE,
        LUATOKEN_PROPERTY,
        LUATOKEN_FUNCTION_DECLARATION,
        LUATOKEN_FUNCTION_CALL,
        LUATOKEN_METHOD,
        LUATOKEN_MACRO,
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

signals:
    void replaceText(QString &text, const QString &kind);

    void insertText(QString &text, const QString &kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void moveUp();

    void moveDown();

    QTableWidget *m_tableWidget = nullptr;
    int m_currentRow{};
    QString m_insertText{};
    QString m_kind{};
    QList<QString> m_kindList{};
};

class TooltipHover final : public QWidget {
    Q_OBJECT

public:
    explicit TooltipHover(QWidget *parent = nullptr);

    ~TooltipHover() override = default;

    void showTooltip(const QString &message);

    void hideTooltip();

signals:
    void switchDwell(bool status);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTextBrowser *m_textBrowser = nullptr;
    QPointer<QWidget> m_previousFocus = nullptr;
};

class TooltipPosition final : public QWidget {
    Q_OBJECT

public:
    explicit TooltipPosition(QWidget *parent = nullptr);

    ~TooltipPosition() override = default;

    void showTooltip();

    void hideTooltip();

signals:
    void fillPosition(int x, int y);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTimer *m_timer = nullptr;
    QLabel *m_label = nullptr;
};

class TooltipSignatureHelp final : public QWidget {
    Q_OBJECT

public:
    explicit TooltipSignatureHelp(QWidget *parent = nullptr);

    ~TooltipSignatureHelp() override = default;

    void showTooltip(const QJsonObject &signature);

    void hideTooltip();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLabel *m_label = nullptr;
};

class ScriptEditor final : public QsciScintilla {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void autoPairHandle(int ascii);

private:
    void commentHandle();

    void duplicateHandle();

    QHash<QChar, QChar> m_autoPairHash{};
};

class LuaInterpreter final : public QObject {
    Q_OBJECT

public:
    explicit LuaInterpreter(const QUrl &rootUrl, QObject *parent = nullptr);

    ~LuaInterpreter() override;

    void run(const QString &script) const;

    void debug(const QString &script, const DebugData &debugData) const;

    void hotUpdate(const QString &varScope, const QString &varName, const QString &varValue) const;

private:
    static void luaTerminateHook(lua_State *L, lua_Debug *ar);

    static void luaDebugHook(lua_State *L, lua_Debug *ar);

    void handleError() const;

    lua_State *L = nullptr;
    lua_State *co = nullptr;
};

class ScriptExplorer final : public QTreeView {
    Q_OBJECT

public:
    explicit ScriptExplorer(QWidget *parent = nullptr);

    ~ScriptExplorer() override = default;

    void workspaceOpen(const QUrl &rootUrl);

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

    void scriptOpenInExplorer() const;

    QFileSystemModel *m_model = nullptr;
};

#endif //SCRIPT_H
