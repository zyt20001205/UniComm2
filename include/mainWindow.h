#ifndef UNICOMM_MAINWINDOW_H
#define UNICOMM_MAINWINDOW_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

class QShortcut;

class StructureModule;
class ConfigModule;
class LuaLanguageServer;
class UndoModule;
class PortModule;
class ExplorerModule;
class SendModule;
class DatabaseModule;
class DatatableModule;
class DataplotModule;
class LogModule;
class DiagnosticsModule;
class DebugModule;
class ThreadpoolModule;
class ScriptModule;

class MainWindow final : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &uniqueName = QStringLiteral("MyMainWindow"));

    ~MainWindow() override = default;

    void workspaceOpen();
signals:
    void appendLog(const QString &message, const QString &level);

    void openWorkspace(const QUrl &rootUrl);

    void runThread(const QUrl &scriptUrl, const QString &script);

    void debugThread(const QUrl &scriptUrl, const QString &script);

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

    void workspaceSave(const QString &filePath = QString());

    QJsonObject m_mainConfig{};

    ConfigModule *m_configModule{};
    LuaLanguageServer *m_llsModule{};
    UndoModule *m_undoModule{};
    PortModule *m_portModule{};
    ExplorerModule *m_explorerModule{};
    StructureModule *m_structureModule{};
    SendModule *m_sendModule{};
    DatabaseModule *m_databaseModule{};
    DatatableModule *m_datatableModule{};
    DataplotModule *m_dataplotModule{};
    LogModule *m_logModule{};
    DiagnosticsModule *m_diagnosticsModule{};
    DebugModule *m_debugModule{};
    ThreadpoolModule *m_threadpoolModule{};
    ScriptModule *m_scriptModule{};

    QShortcut *m_openWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceAsShortcut{};
};

#endif //UNICOMM_MAINWINDOW_H
