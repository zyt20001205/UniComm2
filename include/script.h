#ifndef SCRIPT_H
#define SCRIPT_H

#include <QDockWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextBrowser>
#include <QTextEdit>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "port.h"

class Port;

class Script final : public QWidget {
    Q_OBJECT

public:
    explicit Script(QWidget *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

private:
    void uiInit();

    void manualUiInit();

    void scriptRun();

    static int luaPrint(lua_State *L);

    static int luaOpen(lua_State *L);

    static int luaClose(lua_State *L);

    static int luaWrite(lua_State *L);

    static int luaRead(lua_State *L);

    static int luaDelay(lua_State *L);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();

    QWidget *m_scriptWidget = nullptr;
    QTextEdit *m_textEdit = nullptr;
    QListWidget *m_listWidget = nullptr;
    QWidget *m_ctrlWidget = nullptr;
    QHBoxLayout *m_ctrlLayout = nullptr;
    QPushButton *m_runButton = nullptr;
    QPushButton *m_helpButton = nullptr;
    QDialog *m_manualDialog = nullptr;
    QTextBrowser *m_manualTextBrowser = nullptr;

    Port *m_port = nullptr;

private slots:
    void scriptFinished();

private:
    void scriptRunning(QThread *worker);

signals:
    void openPort(int index);

    void closePort(int index);

    void writePort(const QString &command, int index);

    void appendLog(const QString &message, const QString &level);
};

#endif //SCRIPT_H
