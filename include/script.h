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
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "port.h"

class Port;

class ScriptEditor;

class ScriptExplorer;

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

    void scriptConfigSave() const;

    void scriptLoad(const QString &scriptPath);

signals:
    void appendLog(const QString &message, const QString &level);

    void openPort(int index);

    void closePort(int index);

    void writePort(int index, const QString &command, const QString &peerIp);

    void writeDatabase(const QString &key, const QString &value);

private:
    void manualUiInit();

    void scriptRun(const QString &name, const QString &script);

    void scriptRunning(const QString &name, QThread *worker);

    void scriptSave();

    static int luaPrint(lua_State *L);

    static int luaDelay(lua_State *L);

    static int luaInput(lua_State *L);

    static int luaPortOpen(lua_State *L);

    static int luaPortClose(lua_State *L);

    static int luaPortInfo(lua_State *L);

    static int luaPortWrite(lua_State *L);

    static int luaPortRead(lua_State *L);

    static int luaDatabaseWrite(lua_State *L);

    QWidget *m_scriptWidget = nullptr;
    ScriptEditor *m_scriptPlainTextEdit = nullptr;
    QListWidget *m_scriptListWidget = nullptr;
    ScriptExplorer *m_scriptExplorerTreeView = nullptr;
    QWidget *m_ctrlWidget = nullptr;
    QHBoxLayout *m_ctrlLayout = nullptr;
    QDialog *m_manualDialog = nullptr;
    QTextBrowser *m_manualTextBrowser = nullptr;

    Port *m_port = nullptr;
};

class ScriptEditor final : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    ~ScriptEditor() override = default;
};

class ScriptHighlighter final : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit ScriptHighlighter(QTextDocument *parent = nullptr);

    ~ScriptHighlighter() override = default;

protected:
    void highlightBlock(const QString &text) override;
};

class ScriptExplorer final : public QTreeView {
    Q_OBJECT

public:
    explicit ScriptExplorer(QWidget *parent = nullptr);

    using QTreeView::setModel;

    using QTreeView::setRootIndex;

    ~ScriptExplorer() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

    void loadScript(const QString &scriptPath);

    void runScript(const QString &name, const QString &script);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void scriptRun(const QModelIndex &index);

    void scriptLoad(const QModelIndex &index);

    void scriptDelete(const QModelIndex &index);

    QFileSystemModel *m_model = nullptr;
};

#endif //SCRIPT_H
