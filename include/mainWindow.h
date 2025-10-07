#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDockWidget>
#include <QJsonObject>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QWidget>

class Config;
class LuaLanguageServer;
class Port;
class Explorer;
class Send;
class Database;
class Datatable;
class Dataplot;
class Log;
class Diagnostics;
class Debug;
class Threadpool;
class Script;

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

    Config *m_configModule{};
    LuaLanguageServer *m_llsModule{};
    Port *m_portModule{};
    Explorer *m_explorerModule{};
    Send *m_sendModule{};
    Database *m_databaseModule{};
    Datatable *m_datatableModule{};
    Dataplot *m_dataplotModule{};
    Log *m_logModule{};
    Diagnostics *m_diagnosticsModule{};
    Debug *m_debugModule{};
    Threadpool *m_threadpoolModule{};
    Script *m_scriptModule{};

    QAction *m_viewPort{};
    QAction *m_viewExplorer{};
    QAction *m_viewSend{};
    QAction *m_viewDatabase{};
    QAction *m_viewDatatable{};
    QAction *m_viewDataplot{};
    QAction *m_viewLog{};
    QAction *m_viewDiagnostics{};
    QAction *m_viewDebug{};
    QAction *m_viewThreadpool{};
};

#endif //MAINWINDOW_H
