#ifndef UNICOMM_MAINWINDOW_H
#define UNICOMM_MAINWINDOW_H

#include <QJsonObject>
#include <QQuickWidget>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

class QQuickWidget;
class QComboBox;
class QShortcut;

class ConfigManager;
class LuaLanguageServer;

class BreakpointModule;
class DebugModule;
class DiagnosticsModule;
class ExplorerModule;
class LauncherModule;
class LogModule;
class NuspellModule;
class StructureModule;
class ThreadpoolModule;

class UndoModule;
class SettingModule;
class ScriptModule;
class PortModule;
class SendModule;
class DatabaseModule;
class DatatableModule;
class DataplotModule;

class MainWindow final : public KDDockWidgets::QtWidgets::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, const QString &uniqueName = QStringLiteral("MyMainWindow"));

    ~MainWindow() override = default;

    void propertySet();

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void overlayTransparent(bool status) const;

    Q_INVOKABLE void quit();

signals:
    void appendLog(const QString &message, const QString &level);

    void startThread(const QUrl &scriptUrl, int mode, QString &threadId);

protected:
    void closeEvent(QCloseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    void moduleInit();

    void menuInit();

    void shortcutInit();

    void layoutInit();

    void overlayInit();

    void mainConfigSave();

    void workspaceOpen();

    void workspaceSave(QString filePath = QString());

    QJsonObject m_mainConfig{};
    QQuickWidget *m_overlay{};
    QObject *m_closeDialog{};
    bool m_askForSaving = true;

    ConfigManager *m_configManager{};
    LuaLanguageServer *m_luals{};

    BreakpointModule *m_breakpointModule{};
    DatabaseModule *m_databaseModule{};
    DataplotModule *m_dataplotModule{};
    DatatableModule *m_datatableModule{};
    DebugModule *m_debugModule{};
    DiagnosticsModule *m_diagnosticsModule{};
    ExplorerModule *m_explorerModule{};
    LauncherModule *m_launcherModule{};
    LogModule *m_logModule{};
    NuspellModule *m_nuspellModule{};
    PortModule *m_portModule{};
    ScriptModule *m_scriptModule{};
    SendModule *m_sendModule{};
    SettingModule *m_settingModule{};
    StructureModule *m_structureModule{};
    ThreadpoolModule *m_threadpoolModule{};
    UndoModule *m_undoModule{};

    QShortcut *m_openWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceShortcut{};
    QShortcut *m_saveWorkspaceAsShortcut{};

    QComboBox *m_scriptComboBox{};
};

#endif //UNICOMM_MAINWINDOW_H
