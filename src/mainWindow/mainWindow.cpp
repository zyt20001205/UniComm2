#include "mainWindow/mainWindow.h"

#include <QCameraDevice>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QMediaDevices>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QShortcut>
#include <QStandardPaths>
#include <QStatusBar>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <kddockwidgets/LayoutSaver.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include "configManager.h"
#include "globals.h"
#include "systemModule.h"
#include "logModule.h"
#include "undoModule.h"
#include "dataModule/databaseModule.h"
#include "dataModule/dataplotModule.h"
#include "dataModule/datatableModule.h"
#include "luaModule/luaInterpreter.h"
#include "luaModule/luaLanguageServer.h"
#include "mainWindow/menuModule.h"
#include "mainWindow/statusModule.h"
#include "portModule/portModule.h"
#include "portModule/sendModule.h"
#include "scriptModule/nuspellModule.h"
#include "scriptModule/scriptModule.h"
#include "scriptModule/codeEditor/explorerModule.h"
#include "scriptModule/codeDebug/breakpointModule.h"
#include "scriptModule/codeDebug/debugModule.h"
#include "scriptModule/codeDebug/threadpoolModule.h"
#include "scriptModule/codeAnalysis/diagnosticsModule.h"
#include "scriptModule/codeAnalysis/structureModule.h"
#include "scriptModule/codeDebug/watchModule.h"

// public
MainWindow::MainWindow(QWidget *parent, const QString &uniqueName)
    : KDDockWidgets::QtWidgets::MainWindow(uniqueName, KDDockWidgets::MainWindowOption_None, parent) {
    // mainWindow ui init
    g_mainWindow = this;
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget::setWindowTitle("UniComm");
    QWidget::setWindowIcon(QIcon(":/icon/icon.ico"));
    QWidget::showMaximized();

    moduleInit();
    shortcutInit();
    overlayInit();
    layoutInit();

    // logging
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] main window created").arg(timestamp);

    // preload multimedia to avoid lagging on port selection
    QThread *worker = QThread::create([] {
        QMediaDevices::videoInputs();
    });
    worker->start();
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
}

MainWindow::~MainWindow() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] main window destructed").arg(timestamp);
}

void MainWindow::propertySet() {
    m_overlay->rootContext()->setContextProperty("mainWindow", this);
    m_overlay->rootContext()->setContextProperty("breakpointModule", m_breakpointModule);
    m_overlay->rootContext()->setContextProperty("databaseModule", m_databaseModule);
    // m_overlay->rootContext()->setContextProperty("dataplotModule", m_dataplotModule);
    m_overlay->rootContext()->setContextProperty("datatableModule", m_datatableModule);
    m_overlay->rootContext()->setContextProperty("debugModule", m_debugModule);
    m_overlay->rootContext()->setContextProperty("diagnosticsModule", m_diagnosticsModule);
    m_overlay->rootContext()->setContextProperty("explorerModule", m_explorerModule);
    m_overlay->rootContext()->setContextProperty("logModule", m_logModule);
    m_overlay->rootContext()->setContextProperty("menuModule", m_menuModule);
    m_overlay->rootContext()->setContextProperty("portModule", m_portModule);
    // m_overlay->rootContext()->setContextProperty("statusModule", m_statusModule);
    // m_overlay->rootContext()->setContextProperty("structureModule", m_structureModule);
    m_overlay->rootContext()->setContextProperty("scriptModule", m_scriptModule);
    // m_overlay->rootContext()->setContextProperty("sendModule", m_sendModule);
    m_overlay->rootContext()->setContextProperty("systemModule", m_systemModule);
    m_overlay->rootContext()->setContextProperty("threadpoolModule", m_threadpoolModule);
    m_overlay->rootContext()->setContextProperty("watchModule", m_watchModule);

    m_overlay->rootContext()->setContextProperty("breakpointModuleAction", QVariant::fromValue(m_breakpointModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("databaseModuleAction", QVariant::fromValue(m_databaseModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("dataplotModuleAction", QVariant::fromValue(m_dataplotModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("datatableModuleAction", QVariant::fromValue(m_datatableModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("debugModuleAction", QVariant::fromValue(m_debugModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("diagnosticsModuleAction", QVariant::fromValue(m_diagnosticsModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("explorerModuleAction", QVariant::fromValue(m_explorerModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("logModuleAction", QVariant::fromValue(m_logModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("portModuleAction", QVariant::fromValue(m_portModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("sendModuleAction", QVariant::fromValue(m_sendModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("structureModuleAction", QVariant::fromValue(m_structureModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("threadpoolModuleAction", QVariant::fromValue(m_threadpoolModule->toggleAction()));
    m_overlay->rootContext()->setContextProperty("watchModuleAction", QVariant::fromValue(m_watchModule->toggleAction()));
}

void MainWindow::propertyGet(const QVariantMap &objects) {
    m_closeDialog = qvariant_cast<QObject *>(objects["mainWindowCloseDialog"]);
    m_quitDialog = qvariant_cast<QObject *>(objects["mainWindowQuitDialog"]);

    const QVariantMap lualsObjects = {
        {"lualsProgressDialog", objects["lualsProgressDialog"]}
    };
    m_luals->propertySet(lualsObjects);

    const QVariantMap breakpointObjects = {
        {"breakpointModuleLineMenu", objects["breakpointModuleLineMenu"]},
        {"breakpointModuleFileMenu", objects["breakpointModuleFileMenu"]},
        {"breakpointModuleRootMenu", objects["breakpointModuleRootMenu"]}
    };
    m_breakpointModule->propertySet(breakpointObjects);

    const QVariantMap databaseObjects = {
        {"databaseModuleEditDialog", objects["databaseModuleEditDialog"]},
        {"databaseModuleTableMenu", objects["databaseModuleTableMenu"]},
        {"databaseModuleRootMenu", objects["databaseModuleRootMenu"]}
    };
    m_databaseModule->propertySet(databaseObjects);

    const QVariantMap dataplotObjects = {
        {"dataplotModuleRootMenu", objects["dataplotModuleRootMenu"]}
    };
    m_dataplotModule->propertySet(dataplotObjects);

    const QVariantMap datatableObjects = {
        {"datatableModuleEditDialog", objects["datatableModuleEditDialog"]},
        {"datatableModuleTableMenu", objects["datatableModuleTableMenu"]},
        {"datatableModuleRootMenu", objects["datatableModuleRootMenu"]}
    };
    m_datatableModule->propertySet(datatableObjects);

    const QVariantMap debugObjects = {
        {"debugModuleErrorDialog", objects["debugModuleErrorDialog"]}
    };
    m_debugModule->propertySet(debugObjects);

    const QVariantMap diagnosticsObjects = {
        {"diagnosticsModuleDiagnosticMenu", objects["diagnosticsModuleDiagnosticMenu"]}
    };
    m_diagnosticsModule->propertySet(diagnosticsObjects);

    const QVariantMap explorerObjects = {
        {"explorerModuleScriptMenu", objects["explorerModuleScriptMenu"]},
        {"explorerModuleFolderMenu", objects["explorerModuleFolderMenu"]},
        {"explorerModuleRootMenu", objects["explorerModuleRootMenu"]}
    };
    m_explorerModule->propertySet(explorerObjects);

    const QVariantMap logObjects = {
        {"logModuleEmptyDialog", objects["logModuleEmptyDialog"]},
        {"logModuleHeightDialog", objects["logModuleHeightDialog"]},
        {"logModuleLinkMenu", objects["logModuleLinkMenu"]}
    };
    m_logModule->propertySet(logObjects);

    const QVariantMap menuObjects = {
        {"menuModuleFileMenu", objects["menuModuleFileMenu"]},
        {"menuModuleViewMenu", objects["menuModuleViewMenu"]}
    };
    m_menuModule->propertySet(menuObjects);

    const QVariantMap portObjects = {
        {"portModuleTableMenu", objects["portModuleTableMenu"]},
        {"portModuleRootMenu", objects["portModuleRootMenu"]}
    };
    m_portModule->propertySet(portObjects);

    const QVariantMap scriptObjects = {
        {"breakpointModuleEditDialog", objects["breakpointModuleEditDialog"]},
        {"systemModulePermissionDialog", objects["systemModulePermissionDialog"]},
        {"scriptModuleEditorMenu", objects["scriptModuleEditorMenu"]},
        {"scriptModuleCompletionToolTip", objects["scriptModuleCompletionToolTip"]},
        {"scriptModuleCompletionTableView", objects["scriptModuleCompletionTableView"]},
        {"scriptModuleCompletionDetailTableView", objects["scriptModuleCompletionDetailTableView"]},
        {"scriptModuleDwellToolTip", objects["scriptModuleDwellToolTip"]},
        {"scriptModuleDwellDiagnosticTextArea", objects["scriptModuleDwellDiagnosticTextArea"]},
        {"scriptModuleDwellHoverTextArea", objects["scriptModuleDwellHoverTextArea"]},
        {"scriptModuleDwellCodeActionMenu", objects["scriptModuleDwellCodeActionMenu"]},
        {"scriptModuleDwellSuggestionMenu", objects["scriptModuleDwellSuggestionMenu"]},
        {"scriptModuleNavigationToolTip", objects["scriptModuleNavigationToolTip"]},
        {"scriptModuleNavigationTableView", objects["scriptModuleNavigationTableView"]},
        {"scriptModuleNavigationDetailLabel", objects["scriptModuleNavigationDetailLabel"]},
        {"scriptModuleSignatureToolTip", objects["scriptModuleSignatureToolTip"]},
        {"scriptModuleSignatureLabel", objects["scriptModuleSignatureLabel"]},
        {"scriptModuleToolTip", objects["scriptModuleToolTip"]}
    };
    m_scriptModule->propertySet(scriptObjects);

    const QVariantMap sendObjects = {
        //
    };
    m_sendModule->propertySet(sendObjects);

    const QVariantMap statusObjects = {
        {"statusModuleEolModeMenu", objects["statusModuleEolModeMenu"]}
    };
    m_statusModule->propertySet(statusObjects);

    const QVariantMap structureObjects = {
        {"structureModuleRootMenu", objects["structureModuleRootMenu"]}
    };
    m_structureModule->propertySet(structureObjects);

    const QVariantMap systemObjects = {
        {"mainWindowBusyDialog", objects["mainWindowBusyDialog"]},
        {"systemModuleErrorDialog", objects["systemModuleErrorDialog"]}
    };
    m_systemModule->propertySet(systemObjects);

    const QVariantMap threadpoolObjects = {
        {"mainItem", objects["mainItem"]},
        {"threadpoolModuleErrorDialog", objects["threadpoolModuleErrorDialog"]},
        {"threadpoolModuleThreadMenu", objects["threadpoolModuleThreadMenu"]}
    };
    m_threadpoolModule->propertySet(threadpoolObjects);

    const QVariantMap watchObjects = {
        {"watchModuleExpressionMenu", objects["watchModuleExpressionMenu"]},
        {"watchModuleValueMenu", objects["watchModuleValueMenu"]},
        {"watchModuleRootMenu", objects["watchModuleRootMenu"]}
    };
    m_watchModule->propertySet(watchObjects);
}

void MainWindow::overlayFocus(const bool status) const {
    QTimer::singleShot(0, [this, status] {
        if (m_overlay->flags().testFlag(Qt::WindowDoesNotAcceptFocus) == !status) return;
        m_overlay->setFlag(Qt::WindowDoesNotAcceptFocus, !status);
        m_overlay->hide();
        m_overlay->showMaximized();
    });
}

void MainWindow::overlayTransparent(const bool status) const {
    QTimer::singleShot(0, [this, status] {
        if (m_overlay->flags().testFlag(Qt::WindowTransparentForInput) == status) return;
        m_overlay->setFlag(Qt::WindowTransparentForInput, status);
        m_overlay->hide();
        m_overlay->showMaximized();
    });
}

void MainWindow::quit() {
    workspaceSave();

    QMetaObject::invokeMethod(m_quitDialog, "open");
    constexpr float total = 2;
    int current = 0;
    // quit modules
    current++;
    m_quitDialog->setProperty("primaryLog", tr("Waiting for threadpool module..."));
    m_quitDialog->setProperty("primaryProgress", static_cast<float>(current) / total);
    m_threadpoolModule->quit();
    current++;
    m_quitDialog->setProperty("primaryLog", tr("Waiting for lua language server module..."));
    m_quitDialog->setProperty("primaryProgress", static_cast<float>(current) / total);
    m_luals->quit();

    m_askForSaving = false;
    m_overlay->close();
    close();
}

void MainWindow::terminate() {
    m_askForSaving = false;
    m_overlay->close();
    close();
}

void MainWindow::workspaceOpen() {
    const QString workspaceDir = QFileDialog::getExistingDirectory(
        g_mainWindow,
        tr("Open Workspace"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (workspaceDir.isEmpty()) return;
    if (g_workspaceUrl == QUrl::fromLocalFile(workspaceDir)) {
        qDebug() << "same as prev workspace";
        return;
    }
    workspaceSave();
    g_workspaceUrl = QUrl::fromLocalFile(workspaceDir);
    // write to main config
    const QJsonObject json{
        {"version", "1.0.0"},
        {"workspace", g_workspaceUrl.toString()},
    };
    const QJsonDocument doc(json);
    QFile mainConfig(QDir::current().filePath("config.json"));
    mainConfig.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    mainConfig.write(doc.toJson(QJsonDocument::Indented));
    mainConfig.close();
    // restart main process
    QProcess::startDetached(QCoreApplication::applicationFilePath());
    m_askForSaving = false;
    QApplication::quit();
}

void MainWindow::workspaceSave(const QUrl &configUrl) {
    m_scriptModule->scriptConfigSave();
    BreakpointModule::breakpointConfigSave();
    DatabaseModule::databaseConfigSave();
    DatatableModule::datatableConfigSave();
    m_logModule->logConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_watchModule->watchConfigSave();
    mainConfigSave();
    m_configManager->workspaceConfigSave(configUrl);
}

void MainWindow::quitTrack(const float secondaryProgress, const QString &secondaryLog) const {
    m_quitDialog->setProperty("secondaryProgress", secondaryProgress);
    m_quitDialog->setProperty("secondaryLog", secondaryLog);
}

// protected
void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_askForSaving) {
        event->ignore();
        QMetaObject::invokeMethod(m_closeDialog, "open");
    } else {
        event->accept();
    }
}

// private
void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_configManager = new ConfigManager(this);
    m_luals = new LuaLanguageServer(this);

    m_breakpointModule = new BreakpointModule();
    m_databaseModule = new DatabaseModule();
    m_dataplotModule = new DataplotModule();
    m_datatableModule = new DatatableModule();
    m_debugModule = new DebugModule();
    m_diagnosticsModule = new DiagnosticsModule();
    m_explorerModule = new ExplorerModule();
    m_logModule = new LogModule();
    m_nuspellModule = new NuspellModule(this);
    m_menuModule = new MenuModule(this);
    m_portModule = new PortModule();
    m_scriptModule = new ScriptModule();
    m_sendModule = new SendModule();
    m_statusModule = new StatusModule(this);
    m_structureModule = new StructureModule();
    m_systemModule = new SystemModule();
    m_threadpoolModule = new ThreadpoolModule();
    m_undoModule = new UndoModule(this);
    m_watchModule = new WatchModule();

    m_mainConfig = g_workspaceConfig["mainConfig"].toObject();
    g_database = m_databaseModule;
    g_datatable = m_datatableModule;
    g_dataplot = m_dataplotModule;
    g_nuspell = m_nuspellModule;
    g_port = m_portModule;
    g_script = m_scriptModule;
    g_undo = m_undoModule;

    connect(this, &MainWindow::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_configManager, &ConfigManager::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_luals, &LuaLanguageServer::notificationPublishDiagnostics, m_scriptModule, &ScriptModule::diagnosticsNotification);
    connect(m_luals, &LuaLanguageServer::notificationPublishDiagnostics, m_diagnosticsModule, &DiagnosticsModule::diagnosticsNotification);
    connect(m_luals, &LuaLanguageServer::responseCodeAction, m_scriptModule, &ScriptModule::responseCodeAction);
    connect(m_luals, &LuaLanguageServer::responseCompletion, m_scriptModule, &ScriptModule::completionResponse);
    connect(m_luals, &LuaLanguageServer::responseDefinition, m_scriptModule, &ScriptModule::definitionResponse);
    connect(m_luals, &LuaLanguageServer::responseDocumentHighlight, m_scriptModule, &ScriptModule::documentHighlightResponse);
    connect(m_luals, &LuaLanguageServer::responseDocumentSymbol, m_scriptModule, &ScriptModule::documentSymbolResponse);
    connect(m_luals, &LuaLanguageServer::responseDocumentSymbol, m_structureModule, &StructureModule::documentSymbolResponse);
    connect(m_luals, &LuaLanguageServer::responseFoldingRange, m_scriptModule, &ScriptModule::foldingRangeResponse);
    connect(m_luals, &LuaLanguageServer::responseFormatting, m_scriptModule, &ScriptModule::formattingResponse);
    connect(m_luals, &LuaLanguageServer::responseHover, m_scriptModule, &ScriptModule::hoverResponse);
    connect(m_luals, &LuaLanguageServer::responseImplementation, m_scriptModule, &ScriptModule::implementationResponse);
    connect(m_luals, &LuaLanguageServer::responseOnTypeFormatting, m_scriptModule, &ScriptModule::onTypeFormattingResponse);
    connect(m_luals, &LuaLanguageServer::responseRangeFormatting, m_scriptModule, &ScriptModule::rangeFormattingResponse);
    connect(m_luals, &LuaLanguageServer::responseReferences, m_scriptModule, &ScriptModule::referencesResponse);
    connect(m_luals, &LuaLanguageServer::responseSemanticTokens, m_scriptModule, &ScriptModule::semanticTokensResponse);
    connect(m_luals, &LuaLanguageServer::responseSignatureHelp, m_scriptModule, &ScriptModule::signatureHelpResponse);
    connect(m_luals, &LuaLanguageServer::responseTypeDefinition, m_scriptModule, &ScriptModule::typeDefinitionResponse);

    connect(m_breakpointModule, &BreakpointModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_breakpointModule, &BreakpointModule::addMarker, m_scriptModule, &ScriptModule::markerAdd);
    connect(m_breakpointModule, &BreakpointModule::deleteMarker, m_scriptModule, &ScriptModule::markerDelete);

    connect(m_databaseModule, &DatabaseModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_datatableModule, &DatatableModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_debugModule, &DebugModule::getIndex, m_scriptModule, &ScriptModule::indexGet);
    connect(m_debugModule, &DebugModule::addMarker, m_scriptModule, &ScriptModule::markerAdd);
    connect(m_debugModule, &DebugModule::setState, m_threadpoolModule, &ThreadpoolModule::stateSet);

    connect(m_diagnosticsModule, &DiagnosticsModule::setIndex, m_scriptModule, &ScriptModule::indexSet);
    connect(m_diagnosticsModule, &DiagnosticsModule::fillIndicator, m_scriptModule, &ScriptModule::indicatorFill);

    connect(m_explorerModule, &ExplorerModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_explorerModule, &ExplorerModule::startThread, m_threadpoolModule,
            qOverload<const QUrl &, const int, const int, const int, const int, const int>(&ThreadpoolModule::threadStart));

    connect(m_nuspellModule, &NuspellModule::responseSpellCheck, m_scriptModule, &ScriptModule::spellCheckResponse);

    connect(m_portModule, &PortModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_scriptModule, &ScriptModule::requestJson, m_luals, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &ScriptModule::notificationJson, m_luals, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &ScriptModule::requestSpellCheck, m_nuspellModule, &NuspellModule::spellCheckRequest);
    connect(m_scriptModule, &ScriptModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_scriptModule, &ScriptModule::openWorkspace, this, &MainWindow::workspaceOpen);
    connect(m_scriptModule, &ScriptModule::startThread, m_threadpoolModule,
            qOverload<const QUrl &, const int, const int, const int, const int, const int>(&ThreadpoolModule::threadStart));
    connect(m_scriptModule, &ScriptModule::focusScript, m_statusModule, &StatusModule::scriptFocus);
    connect(m_scriptModule, &ScriptModule::focusScript, m_structureModule, &StructureModule::scriptFocus);
    connect(m_scriptModule, &ScriptModule::changeSelection, m_statusModule, &StatusModule::selectionChange);
    connect(m_scriptModule, &ScriptModule::insertBreakpoint, m_breakpointModule, &BreakpointModule::breakpointInsert);
    connect(m_scriptModule, &ScriptModule::removeBreakpoint, m_breakpointModule, &BreakpointModule::breakpointRemove);

    connect(m_structureModule, &StructureModule::addMarker, m_scriptModule, &ScriptModule::markerAdd);

    connect(m_systemModule, &SystemModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_systemModule, &SystemModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_systemModule, &SystemModule::notificationJson, m_luals, &LuaLanguageServer::jsonNotification);

    connect(m_threadpoolModule, &ThreadpoolModule::trackQuit, this, &MainWindow::quitTrack);
    connect(m_threadpoolModule, &ThreadpoolModule::refreshThread, m_statusModule, &StatusModule::threadRefresh);
    connect(m_threadpoolModule, &ThreadpoolModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_threadpoolModule, &ThreadpoolModule::addMarker, m_scriptModule, &ScriptModule::markerAdd);
    connect(m_threadpoolModule, &ThreadpoolModule::deleteMarker, m_scriptModule, &ScriptModule::markerDelete);
    connect(m_threadpoolModule, &ThreadpoolModule::insertCallStack, m_debugModule, &DebugModule::callStackInsert);
    connect(m_threadpoolModule, &ThreadpoolModule::startDebug, m_debugModule, &DebugModule::debugStart);
    connect(m_threadpoolModule, &ThreadpoolModule::stopDebug, m_debugModule, &DebugModule::debugStop);
    connect(m_threadpoolModule, &ThreadpoolModule::appendLog, m_logModule, &LogModule::logAppend);
}

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_workspaceConfig["shortcutConfig"].toObject();
    const auto *maximizeShortcut = new QShortcut(QKeySequence("F11"), this); // NOLINT
    connect(maximizeShortcut, &QShortcut::activated, this, &MainWindow::maximizeToggle);
    const auto *openWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["openWorkspace"].toString()), this); // NOLINT
    connect(openWorkspaceShortcut, &QShortcut::activated, this, &MainWindow::workspaceOpen);
    const auto *saveWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspace"].toString()), this); // NOLINT
    connect(saveWorkspaceShortcut, &QShortcut::activated, this, [this] { workspaceSave(); });
    const auto *saveWorkspaceAsShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspaceAs"].toString()), this); // NOLINT
    connect(saveWorkspaceAsShortcut, &QShortcut::activated, this, [this] {
        const QString filePath = QFileDialog::getSaveFileName(
            nullptr,
            tr("Save Workspace As"),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/config",
            "JSON File (*.json)"
        );
        if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
            workspaceSave(filePath);
        }
    });
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "shortcut initialized");
}

// // setting menu
// {
//     auto *settingAction = new QAction(tr("Setting"), this); // NOLINT
//     toolBar->addAction(settingAction);
//     connect(settingAction, &QAction::triggered, this, [this] {
//         const QJsonObject logConfig = g_workspaceConfig["logConfig"].toObject();
//         const QJsonObject scriptConfig = g_workspaceConfig["scriptConfig"].toObject();
//         const QJsonObject settingConfig = {
//             {"fontFamilyLog", logConfig["fontFamily"].toString()},
//             {"fontSizeLog", logConfig["fontSize"].toInt()},
//             {"fontFamilyScript", scriptConfig["fontFamily"].toString()},
//             {"fontSizeScript", scriptConfig["fontSize"].toInt()},
//             {"indicatorErrorStyleScript", scriptConfig["indicatorErrorStyle"].toInt()},
//             {"indicatorErrorColorScript", scriptConfig["indicatorErrorColor"].toString()},
//             {"indicatorWarningStyleScript", scriptConfig["indicatorWarningStyle"].toInt()},
//             {"indicatorWarningColorScript", scriptConfig["indicatorWarningColor"].toString()},
//             {"indicatorInfoStyleScript", scriptConfig["indicatorInfoStyle"].toInt()},
//             {"indicatorInfoColorScript", scriptConfig["indicatorInfoColor"].toString()},
//             {"indicatorHintStyleScript", scriptConfig["indicatorHintStyle"].toInt()},
//             {"indicatorHintColorScript", scriptConfig["indicatorHintColor"].toString()},
//             {"indicatorHighlightStyleScript", scriptConfig["indicatorHighlightStyle"].toInt()},
//             {"indicatorHighlightColorScript", scriptConfig["indicatorHighlightColor"].toString()},
//             {"indicatorReadStyleScript", scriptConfig["indicatorReadStyle"].toInt()},
//             {"indicatorReadColorScript", scriptConfig["indicatorReadColor"].toString()},
//             {"indicatorWriteStyleScript", scriptConfig["indicatorWriteStyle"].toInt()},
//             {"indicatorWriteColorScript", scriptConfig["indicatorWriteColor"].toString()},
//             {"indicatorSearchStyleScript", scriptConfig["indicatorSearchStyle"].toInt()},
//             {"indicatorSearchColorScript", scriptConfig["indicatorSearchColor"].toString()},
//             {"indicatorSelectionStyleScript", scriptConfig["indicatorSelectionStyle"].toInt()},
//             {"indicatorSelectionColorScript", scriptConfig["indicatorSelectionColor"].toString()},
//             {"indicatorHyperlinkStyleScript", scriptConfig["indicatorHyperlinkStyle"].toInt()},
//             {"indicatorHyperlinkColorScript", scriptConfig["indicatorHyperlinkColor"].toString()},
//             {"markerBreakpointStyleScript", scriptConfig["markerBreakpointStyle"].toInt()},
//             {"markerBreakpointBackgroundScript", scriptConfig["markerBreakpointBackground"].toString()},
//             {"markerBreakpointForegroundScript", scriptConfig["markerBreakpointForeground"].toString()},
//             {"markerDebugStyleScript", scriptConfig["markerDebugStyle"].toInt()},
//             {"markerDebugBackgroundScript", scriptConfig["markerDebugBackground"].toString()},
//             {"markerDebugForegroundScript", scriptConfig["markerDebugForeground"].toString()}
//         };
//         m_settingModule->settingImport(settingConfig);
//         if (m_settingModule->exec() == QDialog::Accepted) {
//             workspaceSave();
//         }
//     });
// }
// // logging
// QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
// qDebug() << QString("[%1] %2").arg(timestamp, "menu initialized");

void MainWindow::layoutInit() {
    auto *toolBar = new QToolBar(); // NOLINT
    addToolBar(Qt::TopToolBarArea, toolBar);
    toolBar->addWidget(m_menuModule);
    toolBar->setMovable(false);

    auto *statusBar = this->statusBar();
    statusBar->addWidget(m_statusModule, 1);

    addDockWidget(m_scriptModule->welcomePage(), KDDockWidgets::Location_OnRight);
    // left
    addDockWidget(m_portModule, KDDockWidgets::Location_OnLeft, nullptr, KDDockWidgets::InitialOption(KDDockWidgets::Size(400, 0)));
    addDockWidget(m_sendModule, KDDockWidgets::Location_OnBottom, m_portModule);
    addDockWidget(m_explorerModule, KDDockWidgets::Location_OnBottom, m_sendModule);
    addDockWidget(m_structureModule, KDDockWidgets::Location_OnBottom, m_explorerModule);
    // right
    addDockWidget(m_breakpointModule, KDDockWidgets::Location_OnRight, nullptr, KDDockWidgets::InitialOption(KDDockWidgets::Size(300, 0)));
    addDockWidget(m_debugModule, KDDockWidgets::Location_OnBottom, m_breakpointModule);
    addDockWidget(m_watchModule, KDDockWidgets::Location_OnBottom, m_debugModule);
    // bottom
    addDockWidget(m_logModule, KDDockWidgets::Location_OnBottom, nullptr, KDDockWidgets::InitialOption(KDDockWidgets::Size(0, 200)));
    m_logModule->addDockWidgetAsTab(m_diagnosticsModule, KDDockWidgets::InitialVisibilityOption::PreserveCurrentTab);
    addDockWidget(m_databaseModule, KDDockWidgets::Location_OnRight, m_logModule, KDDockWidgets::InitialOption(KDDockWidgets::Size(600, 0)));
    m_databaseModule->addDockWidgetAsTab(m_datatableModule, KDDockWidgets::InitialVisibilityOption::PreserveCurrentTab);
    m_databaseModule->addDockWidgetAsTab(m_dataplotModule, KDDockWidgets::InitialVisibilityOption::PreserveCurrentTab);
    addDockWidget(m_threadpoolModule, KDDockWidgets::Location_OnRight, m_databaseModule, KDDockWidgets::InitialOption(KDDockWidgets::Size(300, 0)));

    if (!m_mainConfig["state"].toString().isEmpty()) {
        const QByteArray layoutData = QByteArray::fromBase64(m_mainConfig["state"].toString().toLatin1());
        KDDockWidgets::LayoutSaver layoutSaver;
        layoutSaver.restoreLayout(layoutData);
    }
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "layout initialized");
}

void MainWindow::overlayInit() {
    m_overlay = new QQuickView();
    m_overlay->setColor(Qt::transparent);
    m_overlay->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    propertySet();
    m_overlay->setSource(QUrl("qrc:/qml/mainWindow/mainWindow.qml"));
    m_overlay->showMaximized();
}

void MainWindow::mainConfigSave() {
    const KDDockWidgets::LayoutSaver layoutSaver;
    const QByteArray layoutData = layoutSaver.serializeLayout();
    m_mainConfig["state"] = QString(layoutData.toBase64());
    g_workspaceConfig["mainConfig"] = m_mainConfig;
}

void MainWindow::maximizeToggle() {
    if (isMaximized()) showNormal();
    else showMaximized();
}
