#ifndef UNICOMM_MAINWINDOW_H
#define UNICOMM_MAINWINDOW_H

#include <QApplication>
#include <QDateTime>
#include <QDockWidget>
#include <QJsonObject>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QStyleFactory>
#include <QWidget>

class Config;
class LuaLanguageServer;
class UndoModule;
class PortModule;
class Explorer;
class SendModule;
class DatabaseModule;
class DatatableModule;
class DataplotModule;
class Log;
class Diagnostics;
class Debug;
class Threadpool;
class ScriptModule;

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
    UndoModule *m_undoModule{};
    PortModule *m_portModule{};
    Explorer *m_explorerModule{};
    SendModule *m_sendModule{};
    DatabaseModule *m_databaseModule{};
    DatatableModule *m_datatableModule{};
    DataplotModule *m_dataplotModule{};
    Log *m_logModule{};
    Diagnostics *m_diagnosticsModule{};
    Debug *m_debugModule{};
    Threadpool *m_threadpoolModule{};
    ScriptModule *m_scriptModule{};

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

#endif //UNICOMM_MAINWINDOW_H
