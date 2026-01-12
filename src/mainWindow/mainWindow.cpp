#include "mainWindow/mainWindow.h"

#include <QCameraDevice>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QMenuBar>
#include <QMessageBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QShortcut>
#include <QStandardPaths>
#include <QStatusBar>
#include <QThread>
#include <QToolBar>
#include <QToolButton>
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
#include "mainWindow/statusModule.h"
#include "portModule/portModule.h"
#include "portModule/sendModule.h"
#include "scriptModule/nuspellModule.h"
#include "scriptModule/scriptModule.h"
#include "scriptModule/codeEditor/editorWidget.h"
#include "scriptModule/codeEditor/explorerModule.h"
#include "scriptModule/codeDebug/breakpointModule.h"
#include "scriptModule/codeDebug/debugModule.h"
#include "scriptModule/codeDebug/threadpoolModule.h"
#include "scriptModule/codeAssistant/diagnosticsModule.h"
#include "scriptModule/codeAssistant/structureModule.h"
#include "scriptModule/codeDebug/watchModule.h"
#include "settingModule/settingModule.h"

// MainWindow public
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
    menuInit();
    layoutInit();
    overlayInit();

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
    m_overlay->rootContext()->setContextProperty("portModule", m_portModule);
    // m_overlay->rootContext()->setContextProperty("statusModule", m_statusModule);
    // m_overlay->rootContext()->setContextProperty("structureModule", m_structureModule);
    m_overlay->rootContext()->setContextProperty("scriptModule", m_scriptModule);
    // m_overlay->rootContext()->setContextProperty("sendModule", m_sendModule);
    m_overlay->rootContext()->setContextProperty("systemModule", m_systemModule);
    m_overlay->rootContext()->setContextProperty("threadpoolModule", m_threadpoolModule);
    m_overlay->rootContext()->setContextProperty("watchModule", m_watchModule);
}

void MainWindow::propertyGet(const QVariantMap &objects) {
    m_closeDialog = qvariant_cast<QObject *>(objects["mainWindowCloseDialog"]);
    m_quitDialog = qvariant_cast<QObject *>(objects["mainWindowQuitDialog"]);

    const QVariantMap lualsObjects = {
        {"lualsProgressDialog",objects["lualsProgressDialog"]}
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
        //
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

    const QVariantMap portObjects = {
        {"portModuleTableMenu", objects["portModuleTableMenu"]},
        {"portModuleRootMenu", objects["portModuleRootMenu"]}
    };
    m_portModule->propertySet(portObjects);

    const QVariantMap scriptObjects = {
        {"scriptModuleEditorMenu", objects["scriptModuleEditorMenu"]},
        {"scriptModuleCompletionToolTip", objects["scriptModuleCompletionToolTip"]},
        {"scriptModuleCompletionTableView", objects["scriptModuleCompletionTableView"]},
        {"scriptModuleCompletionDetailTableView", objects["scriptModuleCompletionDetailTableView"]},
        {"scriptModuleSignatureToolTip", objects["scriptModuleSignatureToolTip"]},
        {"scriptModuleSignatureLabel", objects["scriptModuleSignatureLabel"]}
    };
    m_scriptModule->propertySet(scriptObjects);

    const QVariantMap sendObjects = {
        //
    };
    m_sendModule->propertySet(sendObjects);

    const QVariantMap statusObjects = {
        //
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
        {"threadpoolModuleThreadMenu", objects["threadpoolModuleThreadMenu"]}
    };
    m_threadpoolModule->propertySet(threadpoolObjects);

    const QVariantMap watchObjects = {
        {"watchModuleTableMenu", objects["watchModuleTableMenu"]},
        {"watchModuleRootMenu", objects["watchModuleRootMenu"]}
    };
    m_watchModule->propertySet(watchObjects);
}

void MainWindow::overlayFocus() const {
    m_overlay->setFocus();
}

void MainWindow::overlayTransparent(const bool status) const {
    m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, status);
}

void MainWindow::quit() {
    workspaceSave();

    QMetaObject::invokeMethod(m_quitDialog, "open");
    constexpr float total = 2;
    int current = 0;
    // quit modules
    current ++;
    m_quitDialog->setProperty("primaryLog", tr("Waiting for threadpool module..."));
    m_quitDialog->setProperty("primaryProgress", current / total);
    m_threadpoolModule->quit();
    current ++;
    m_quitDialog->setProperty("primaryLog", tr("Waiting for lua language server module..."));
    m_quitDialog->setProperty("primaryProgress", current / total);
    m_luals->quit();

    m_askForSaving = false;
    close();
}

void MainWindow::terminate() {
    m_askForSaving = false;
    close();
}

void MainWindow::quitTrack(const float secondaryProgress, const QString &secondaryLog) const {
    m_quitDialog->setProperty("secondaryProgress", secondaryProgress);
    m_quitDialog->setProperty("secondaryLog", secondaryLog);
}

// MainWindow protected
void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_askForSaving) {
        event->ignore();
        QMetaObject::invokeMethod(m_closeDialog, "open");
    } else {
        event->accept();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    KDDockWidgets::QtWidgets::MainWindow::resizeEvent(event);
    if (m_overlay) {
        m_overlay->resize(size());
    }
}

// MainWindow private
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
    m_portModule = new PortModule();
    m_scriptModule = new ScriptModule();
    m_sendModule = new SendModule();
    m_settingModule = new SettingModule(this);
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
    connect(m_luals, &LuaLanguageServer::responseDocumentSymbol, m_structureModule, &StructureModule::documentSymbolResponse);
    connect(m_luals, &LuaLanguageServer::responseFoldingRange, m_scriptModule, &ScriptModule::foldingRangeResponse);
    connect(m_luals, &LuaLanguageServer::responseFormatting, m_scriptModule, &ScriptModule::formattingResponse);
    connect(m_luals, &LuaLanguageServer::responseHover, m_scriptModule, &ScriptModule::hoverResponse);
    connect(m_luals, &LuaLanguageServer::responseImplementation, m_scriptModule, &ScriptModule::implementationResponse);
    connect(m_luals, &LuaLanguageServer::responseOnTypeFormatting, m_scriptModule, &ScriptModule::onTypeFormattingResponse);
    connect(m_luals, &LuaLanguageServer::responseReferences, m_scriptModule, &ScriptModule::referencesResponse);
    connect(m_luals, &LuaLanguageServer::responseSemanticTokens, m_scriptModule, &ScriptModule::semanticTokensResponse);
    connect(m_luals, &LuaLanguageServer::responseSignatureHelp, m_scriptModule, &ScriptModule::signatureHelpResponse);
    connect(m_luals, &LuaLanguageServer::responseTypeDefinition, m_scriptModule, &ScriptModule::typeDefinitionResponse);

    connect(m_breakpointModule, &BreakpointModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_breakpointModule, &BreakpointModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);
    connect(m_breakpointModule, &BreakpointModule::removeMarker, m_scriptModule, &ScriptModule::markerRemove);

    connect(m_databaseModule, &DatabaseModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_datatableModule, &DatatableModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_debugModule, &DebugModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_debugModule, &DebugModule::getCursorPosition, m_scriptModule, &ScriptModule::cursorPositionGet);
    connect(m_debugModule, &DebugModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);
    connect(m_debugModule, &DebugModule::setState, m_threadpoolModule, &ThreadpoolModule::stateSet);

    connect(m_diagnosticsModule, &DiagnosticsModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_diagnosticsModule, &DiagnosticsModule::setCursorPosition, m_scriptModule, &ScriptModule::cursorPositionSet);
    connect(m_diagnosticsModule, &DiagnosticsModule::insertIndicator, m_scriptModule, &ScriptModule::indicatorInsert);

    connect(m_explorerModule, &ExplorerModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_explorerModule, &ExplorerModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_explorerModule, &ExplorerModule::startThread, m_threadpoolModule, qOverload<const QUrl &, const int, QString &>(&ThreadpoolModule::threadStart));

    connect(m_nuspellModule, &NuspellModule::responseSpellCheck, m_scriptModule, &ScriptModule::spellCheckResponse);

    connect(m_portModule, &PortModule::appendLog, m_logModule, &LogModule::logAppend);

    connect(m_scriptModule, &ScriptModule::requestJson, m_luals, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &ScriptModule::notificationJson, m_luals, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &ScriptModule::requestSpellCheck, m_nuspellModule, &NuspellModule::spellCheckRequest);
    connect(m_scriptModule, &ScriptModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_scriptModule, &ScriptModule::openWorkspace, this, &MainWindow::workspaceOpen);
    connect(m_scriptModule, &ScriptModule::focusScript, m_statusModule, &StatusModule::scriptFocus);
    connect(m_scriptModule, &ScriptModule::focusScript, m_structureModule, &StructureModule::scriptFocus);
    connect(m_scriptModule, &ScriptModule::positionScript, m_statusModule, &StatusModule::scriptPosition);
    connect(m_scriptModule, &ScriptModule::insertPort, m_portModule, [this] { m_portModule->portInsert(-1, QJsonObject()); });
    connect(m_scriptModule, &ScriptModule::insertDatabase, m_databaseModule, [this] { m_databaseModule->databaseInsert(-1, QString()); });
    connect(m_scriptModule, &ScriptModule::insertDatatable, m_datatableModule, [this] { m_datatableModule->datatableInsert(-1, QString()); });
    connect(m_scriptModule, &ScriptModule::insertBreakpoint, m_breakpointModule, &BreakpointModule::breakpointInsert);
    connect(m_scriptModule, &ScriptModule::removeBreakpoint, m_breakpointModule, &BreakpointModule::breakpointRemove);

    connect(m_structureModule, &StructureModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);

    connect(m_systemModule, &SystemModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_systemModule, &SystemModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_systemModule, &SystemModule::notificationJson, m_luals, &LuaLanguageServer::jsonNotification);

    connect(m_threadpoolModule, &ThreadpoolModule::trackQuit, this, &MainWindow::quitTrack);
    connect(m_threadpoolModule, &ThreadpoolModule::refreshThread, m_statusModule, &StatusModule::threadRefresh);
    connect(m_threadpoolModule, &ThreadpoolModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_threadpoolModule, &ThreadpoolModule::insertMarker, m_scriptModule, &ScriptModule::markerInsert);
    connect(m_threadpoolModule, &ThreadpoolModule::removeMarker, m_scriptModule, &ScriptModule::markerRemove);
    connect(m_threadpoolModule, &ThreadpoolModule::insertCallStack, m_debugModule, &DebugModule::callStackInsert);
    connect(m_threadpoolModule, &ThreadpoolModule::startDebug, m_debugModule, &DebugModule::debugStart);
    connect(m_threadpoolModule, &ThreadpoolModule::stopDebug, m_debugModule, &DebugModule::debugStop);
    connect(m_threadpoolModule, &ThreadpoolModule::listDatabase, m_databaseModule, &DatabaseModule::databaseList);
    connect(m_threadpoolModule, &ThreadpoolModule::writeDatabase, m_databaseModule, &DatabaseModule::databaseWrite);
    connect(m_threadpoolModule, &ThreadpoolModule::listDatatable, m_datatableModule, &DatatableModule::datatableList);
    connect(m_threadpoolModule, &ThreadpoolModule::writeDatatable, m_datatableModule, &DatatableModule::datatableWrite);
    connect(m_threadpoolModule, &ThreadpoolModule::appendLog, m_logModule, &LogModule::logAppend);
    connect(m_threadpoolModule, &ThreadpoolModule::listPort, m_portModule, &PortModule::portList);



    connect(m_settingModule, &SettingModule::reloadLogFont, m_logModule, &LogModule::logFontReload);
    connect(m_settingModule, &SettingModule::saveLogFont, m_logModule, &LogModule::logFontSave);
    connect(m_settingModule, &SettingModule::reloadScriptFont, m_scriptModule, &ScriptModule::scriptFontReload);
    connect(m_settingModule, &SettingModule::saveScriptFont, m_scriptModule, &ScriptModule::scriptFontSave);
    connect(m_settingModule, &SettingModule::reloadScriptIndicator, m_scriptModule, &ScriptModule::scriptIndicatorReload);
    connect(m_settingModule, &SettingModule::saveScriptIndicator, m_scriptModule, &ScriptModule::scriptIndicatorSave);
    connect(m_settingModule, &SettingModule::reloadScriptMarker, m_scriptModule, &ScriptModule::scriptMarkerReload);
    connect(m_settingModule, &SettingModule::saveScriptMarker, m_scriptModule, &ScriptModule::scriptMarkerSave);
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

void MainWindow::menuInit() {
    auto *toolBar = new QToolBar(); // NOLINT
    addToolBar(Qt::TopToolBarArea, toolBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        auto *fileButton = new QToolButton(); // NOLINT
        fileButton->setText(tr("File"));
        fileButton->setMenu(fileMenu);
        fileButton->setPopupMode(QToolButton::InstantPopup);
        toolBar->addWidget(fileButton);

        auto shortcutConfig = g_workspaceConfig["shortcutConfig"].toObject();
        auto *openWorkspaceAction = new QAction(tr("Open Workspace") + "\t" + shortcutConfig["openWorkspace"].toString()); // NOLINT
        fileMenu->addAction(openWorkspaceAction);
        connect(openWorkspaceAction, &QAction::triggered, this, [this] { workspaceOpen(); });
        auto *saveWorkspaceAction = new QAction(tr("Save Workspace") + "\t" + shortcutConfig["saveWorkspace"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAction);
        connect(saveWorkspaceAction, &QAction::triggered, this, [this] { workspaceSave(); });
        auto *saveWorkspaceAsAction = new QAction(tr("Save Workspace As") + "\t" + shortcutConfig["saveWorkspaceAs"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAsAction);
        connect(saveWorkspaceAsAction, &QAction::triggered, this, [this] {
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
    }
    // edit menu
    // {
    //     auto *fileMenu = new QMenu(tr("Edit")); // NOLINT
    //     menuBar->addMenu(fileMenu);
    //     auto *undoAction = new QAction(tr("Undo")); // NOLINT
    //     fileMenu->addAction(undoAction);
    //     auto *redoAction = new QAction(tr("Redo")); // NOLINT
    //     fileMenu->addAction(redoAction);
    // }
    // view menu
    {
        auto *viewMenu = new QMenu(tr("View")); // NOLINT
        auto *viewButton = new QToolButton(); // NOLINT
        viewButton->setText(tr("View"));
        viewButton->setMenu(viewMenu);
        viewButton->setPopupMode(QToolButton::InstantPopup);
        toolBar->addWidget(viewButton);

        viewMenu->addAction(m_portModule->toggleAction());
        m_portModule->toggleAction()->setText(tr("Port"));
        viewMenu->addAction(m_explorerModule->toggleAction());
        m_explorerModule->toggleAction()->setText(tr("Explorer"));
        viewMenu->addAction(m_structureModule->toggleAction());
        m_structureModule->toggleAction()->setText(tr("Structure"));
        viewMenu->addAction(m_sendModule->toggleAction());
        m_sendModule->toggleAction()->setText(tr("Send"));
        viewMenu->addAction(m_portModule->toggleAction());
        m_databaseModule->toggleAction()->setText(tr("Database"));
        viewMenu->addAction(m_databaseModule->toggleAction());
        m_datatableModule->toggleAction()->setText(tr("Data Table"));
        viewMenu->addAction(m_datatableModule->toggleAction());
        m_dataplotModule->toggleAction()->setText(tr("Data Plot"));
        viewMenu->addAction(m_dataplotModule->toggleAction());
        m_logModule->toggleAction()->setText(tr("Log"));
        viewMenu->addAction(m_logModule->toggleAction());
        viewMenu->addAction(m_diagnosticsModule->toggleAction());
        m_diagnosticsModule->toggleAction()->setText(tr("Diagnostics"));
        viewMenu->addAction(m_debugModule->toggleAction());
        m_debugModule->toggleAction()->setText(tr("Debug"));
        viewMenu->addAction(m_threadpoolModule->toggleAction());
        m_threadpoolModule->toggleAction()->setText(tr("Thread Pool"));
        viewMenu->addAction(m_breakpointModule->toggleAction());
        m_breakpointModule->toggleAction()->setText(tr("Breakpoint"));
        viewMenu->addAction(m_watchModule->toggleAction());
        m_watchModule->toggleAction()->setText(tr("Watch"));
    }
    // setting menu
    {
        auto *settingAction = new QAction(tr("Setting"), this); // NOLINT
        toolBar->addAction(settingAction);
        connect(settingAction, &QAction::triggered, this, [this] {
            const QJsonObject logConfig = g_workspaceConfig["logConfig"].toObject();
            const QJsonObject scriptConfig = g_workspaceConfig["scriptConfig"].toObject();
            const QJsonObject settingConfig = {
                {"fontFamilyLog", logConfig["fontFamily"].toString()},
                {"fontSizeLog", logConfig["fontSize"].toInt()},
                {"fontFamilyScript", scriptConfig["fontFamily"].toString()},
                {"fontSizeScript", scriptConfig["fontSize"].toInt()},
                {"indicatorErrorStyleScript", scriptConfig["indicatorErrorStyle"].toInt()},
                {"indicatorErrorColorScript", scriptConfig["indicatorErrorColor"].toString()},
                {"indicatorWarningStyleScript", scriptConfig["indicatorWarningStyle"].toInt()},
                {"indicatorWarningColorScript", scriptConfig["indicatorWarningColor"].toString()},
                {"indicatorInfoStyleScript", scriptConfig["indicatorInfoStyle"].toInt()},
                {"indicatorInfoColorScript", scriptConfig["indicatorInfoColor"].toString()},
                {"indicatorHintStyleScript", scriptConfig["indicatorHintStyle"].toInt()},
                {"indicatorHintColorScript", scriptConfig["indicatorHintColor"].toString()},
                {"indicatorHighlightStyleScript", scriptConfig["indicatorHighlightStyle"].toInt()},
                {"indicatorHighlightColorScript", scriptConfig["indicatorHighlightColor"].toString()},
                {"indicatorReadStyleScript", scriptConfig["indicatorReadStyle"].toInt()},
                {"indicatorReadColorScript", scriptConfig["indicatorReadColor"].toString()},
                {"indicatorWriteStyleScript", scriptConfig["indicatorWriteStyle"].toInt()},
                {"indicatorWriteColorScript", scriptConfig["indicatorWriteColor"].toString()},
                {"indicatorSearchStyleScript", scriptConfig["indicatorSearchStyle"].toInt()},
                {"indicatorSearchColorScript", scriptConfig["indicatorSearchColor"].toString()},
                {"indicatorSelectionStyleScript", scriptConfig["indicatorSelectionStyle"].toInt()},
                {"indicatorSelectionColorScript", scriptConfig["indicatorSelectionColor"].toString()},
                {"indicatorHyperlinkStyleScript", scriptConfig["indicatorHyperlinkStyle"].toInt()},
                {"indicatorHyperlinkColorScript", scriptConfig["indicatorHyperlinkColor"].toString()},
                {"markerBreakpointStyleScript", scriptConfig["markerBreakpointStyle"].toInt()},
                {"markerBreakpointBackgroundScript", scriptConfig["markerBreakpointBackground"].toString()},
                {"markerBreakpointForegroundScript", scriptConfig["markerBreakpointForeground"].toString()},
                {"markerDebugStyleScript", scriptConfig["markerDebugStyle"].toInt()},
                {"markerDebugBackgroundScript", scriptConfig["markerDebugBackground"].toString()},
                {"markerDebugForegroundScript", scriptConfig["markerDebugForeground"].toString()}
            };
            m_settingModule->settingImport(settingConfig);
            if (m_settingModule->exec() == QDialog::Accepted) {
                workspaceSave();
            }
        });
    }
    // separator
    auto *spacer = new QWidget(); // NOLINT
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);
    toolBar->addSeparator();
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "menu initialized");
}

void MainWindow::layoutInit() {
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
    m_overlay = new QQuickWidget(this);
    m_overlay->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_overlay->setClearColor(Qt::transparent);
    m_overlay->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);
    m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_overlay->setFocusPolicy(Qt::NoFocus);
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    m_overlay->setFormat(format);

    propertySet();
    m_overlay->setSource(QUrl("qrc:/qml/mainWindow/mainWindow.qml"));
    m_overlay->resize(size());
    m_overlay->show();
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

void MainWindow::workspaceSave(QString filePath) {
    m_scriptModule->scriptConfigSave();
    m_breakpointModule->breakpointConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_watchModule->watchConfigSave();
    mainConfigSave();
    m_configManager->workspaceConfigSave(filePath);
}
