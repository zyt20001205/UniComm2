#ifndef UNICOMM_MAINWINDOW_H
#define UNICOMM_MAINWINDOW_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

class QComboBox;
class QShortcut;

class StructureModule;
class ConfigModule;
class LuaLanguageServer;
class NuspellModule;
class UndoModule;
class SettingModule;
class ScriptModule;
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

class MainWindow final : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &uniqueName = QStringLiteral("MyMainWindow"));

    ~MainWindow() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

    void startThread(const QUrl &scriptUrl, int mode, QString &threadId);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void moduleInit();

    void menuInit();

    void shortcutInit();

    void layoutInit();

    void mainConfigSave();

    void workspaceOpen();

    void workspaceSave(QString filePath = QString());

    QJsonObject m_mainConfig{};
    bool m_askForSaving = true;

    ConfigModule *m_configModule{};
    LuaLanguageServer *m_llsModule{};
    NuspellModule *m_nuspellModule{};
    UndoModule *m_undoModule{};
    SettingModule *m_settingModule{};
    ScriptModule *m_scriptModule{};
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

    QShortcut *m_openWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceAsShortcut{};

    QComboBox *m_scriptComboBox{};
};

#endif //UNICOMM_MAINWINDOW_H
