#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDockWidget>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QWidget>
#include "config.h"
#include "database.h"
#include "dataplot.h"
#include "datatable.h"
#include "debug.h"
#include "diagnostics.h"
#include "explorer.h"
#include "log.h"
#include "luaLanguageServer.h"
#include "port.h"
#include "script.h"
#include "send.h"
#include "threadpool.h"
#include "utils.h"

inline Database *g_database = nullptr;
inline Datatable *g_datatable = nullptr;
inline Dataplot *g_dataplot = nullptr;
inline Debug *g_debug = nullptr;
inline Log *g_log = nullptr;
inline Port *g_port = nullptr;
inline Script *g_script = nullptr;
inline Threadpool *g_threadpool = nullptr;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace(const QUrl &rootUrl);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void configInit();

    void workspaceInit();

    void moduleInit();

    void menuInit();

    void shortcutInit();

    void layoutInit();

    void mainConfigSave();

    void saveConfig();

    QJsonObject m_mainConfig{};

    Config *m_configModule = nullptr;
    LuaLanguageServer *m_llsModule = nullptr;
    Port *m_portModule = nullptr;
    Explorer *m_explorerModule = nullptr;
    Send *m_sendModule = nullptr;
    Database *m_databaseModule = nullptr;
    Datatable *m_datatableModule = nullptr;
    Dataplot *m_dataplotModule = nullptr;
    Log *m_logModule = nullptr;
    Diagnostics *m_diagnosticsModule = nullptr;
    Debug *m_debugModule = nullptr;
    Threadpool *m_threadpoolModule = nullptr;
    Script *m_scriptModule = nullptr;

    QAction *m_viewPort = nullptr;
    QAction *m_viewExplorer = nullptr;
    QAction *m_viewSend = nullptr;
    QAction *m_viewDatabase = nullptr;
    QAction *m_viewDatatable = nullptr;
    QAction *m_viewDataplot = nullptr;
    QAction *m_viewLog = nullptr;
    QAction *m_viewDiagnostics = nullptr;
    QAction *m_viewDebug = nullptr;
    QAction *m_viewThreadpool = nullptr;
};

#endif //MAINWINDOW_H
