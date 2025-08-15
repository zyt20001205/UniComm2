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
#include <QTextBrowser>
#include <QTextEdit>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>
#include <lua.hpp>
#include "config.h"
#include "port.h"

class Port;

class Script final : public QDockWidget {
    Q_OBJECT

public:
    explicit Script(QObject *parent = nullptr);

    ~Script() override = default;

    void setPort(Port *port) { m_port = port; }

private:
    void uiInit();

    void manualDisplay();

    void scriptRun();

    static int luaPrint(lua_State *L);

    static int luaWrite(lua_State *L);

    static int luaRead(lua_State *L);

    static int luaDelay(lua_State *L);

    QJsonObject m_scriptConfig = g_config["scriptConfig"].toObject();

    QWidget *m_widget = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_scriptWidget = nullptr;
    QVBoxLayout *m_scriptLayout = nullptr;
    QSplitter *m_splitter = nullptr;
    QTextEdit *m_textEdit = nullptr;
    QListWidget *m_listWidget = nullptr;
    QWidget *m_ctrlWidget = nullptr;
    QHBoxLayout *m_ctrlLayout = nullptr;
    QPushButton *m_runButton = nullptr;
    QPushButton *m_helpButton = nullptr;

    Port *m_port = nullptr;

private slots:
    void scriptRunning(QThread *worker);

    void scriptFinished();

signals:
    void start(QThread *worker);

    void writePort(const QString &command, int index);

    void appendLog(const QString &message, const QString &level);
};

#endif //SCRIPT_H
