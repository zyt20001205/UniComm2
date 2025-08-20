#ifndef SCRIPT_H
#define SCRIPT_H

#include <QDockWidget>
#include <QDialog>
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

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

public slots:
    void scriptLoad(const QString &scriptPath);

    void scriptConfigSave() const;

private:
    void manualUiInit();

    void scriptRun();

    void scriptRunning(QThread *worker);

    static int luaPrint(lua_State *L);

    static int luaOpen(lua_State *L);

    static int luaClose(lua_State *L);

    static int luaInfo(lua_State *L);

    static int luaWrite(lua_State *L);

    static int luaRead(lua_State *L);

    static int luaDelay(lua_State *L);

    static int luaInput(lua_State *L);

    QWidget *m_scriptWidget = nullptr;
    ScriptEditor *m_scriptPlainTextEdit = nullptr;
    QListWidget *m_scriptListWidget = nullptr;
    QWidget *m_ctrlWidget = nullptr;
    QHBoxLayout *m_ctrlLayout = nullptr;
    QDialog *m_manualDialog = nullptr;
    QTextBrowser *m_manualTextBrowser = nullptr;

    Port *m_port = nullptr;

private slots:
    void scriptSave();

signals:
    void openPort(int index);

    void closePort(int index);

    void writePort(int index, const QString &command, const QString &peerIp);

    void appendLog(const QString &message, const QString &level);
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

#endif //SCRIPT_H
