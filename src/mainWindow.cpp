#include "mainWindow.h"

#include <QCameraDevice>
#include <QCloseEvent>
#include <QMediaDevices>

#include "configModule.h"
#include "globals.h"
#include "log.h"
#include "undoModule.h"
#include "utils.h"
#include "dataModule/databaseModule.h"
#include "dataModule/dataplotModule.h"
#include "dataModule/datatableModule.h"
#include "luaModule/luaLanguageServer.h"
#include "portModule/portModule.h"
#include "portModule/sendModule.h"
#include "scriptModule/debugModule.h"
#include "scriptModule/diagnosticsModule.h"
#include "scriptModule/explorerModule.h"
#include "scriptModule/scriptModule.h"
#include "scriptModule/threadpoolModule.h"

// MainWindow public
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // mainWindow ui init
    setWindowTitle("UniComm");
    setWindowIcon(QIcon(":/icon/icon.ico"));
    resize(1600, 900);
    setDockNestingEnabled(true);
    setDockOptions(AllowNestedDocks | AllowTabbedDocks | AnimatedDocks);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main window created");

    configInit();
    moduleInit();
    workspaceInit();
    shortcutInit();
    menuInit();
    layoutInit();

    // preload multimedia to avoid lagging on first click
    QTimer::singleShot(0, this, [] {
        QMediaDevices::videoInputs();
    });
}

void MainWindow::workspaceOpen() {
    QUrl rootUrl{};
    // select new root directory
    const QString rootDir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Workspace"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    rootUrl = QUrl::fromLocalFile(rootDir);
    m_mainConfig["workspace"] = rootUrl.toString();
    // check if lua config files exist
    const QString rootPath = rootUrl.toLocalFile();
    if (const QString luarcPath = QDir(rootPath).filePath(".luarc.json"); !QFile::exists(luarcPath)) {
        QFile::copy(":/config/.luarc.json", luarcPath);
        QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                         | QFileDevice::ReadUser | QFileDevice::WriteUser
                                         | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json generated");
    } else if (filehashCalc(":/config/.luarc.json") != filehashCalc(luarcPath)) {
        QFile::remove(luarcPath);
        QFile::copy(":/config/.luarc.json", luarcPath);
        QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                         | QFileDevice::ReadUser | QFileDevice::WriteUser
                                         | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json updated");
    }
    if (const QString libdPath = QDir(rootPath).filePath("lib.d.lua"); !QFile::exists(libdPath)) {
        QFile::copy(":/config/lib.d.lua", libdPath);
        QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                        | QFileDevice::ReadUser | QFileDevice::WriteUser
                                        | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua generated");
    } else if (filehashCalc(":/config/lib.d.lua") != filehashCalc(libdPath)) {
        QFile::remove(libdPath);
        QFile::copy(":/config/lib.d.lua", libdPath);
        QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                        | QFileDevice::ReadUser | QFileDevice::WriteUser
                                        | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua updated");
    }
    emit openWorkspace(rootUrl);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace opened");
}

// MainWindow protected
void MainWindow::closeEvent(QCloseEvent *event) {
    const QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Exit", "Save and exit?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        workspaceSave();
        event->accept();
    } else {
        event->ignore();
    }
}

// MainWindow private
void MainWindow::configInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing config");
    m_configModule = new ConfigModule;
    m_configModule->configInit();
    m_mainConfig = g_config["mainConfig"].toObject();
}

void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_llsModule = new LuaLanguageServer(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "lls module initialized");

    m_undoModule = new UndoModule(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "undo module initialized");

    m_portModule = new PortModule();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_portModule);
    m_portModule->setObjectName("portModule");
    connect(m_portModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_portModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_explorerModule = new ExplorerModule();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_explorerModule);
    m_explorerModule->setObjectName("explorerModule");
    connect(m_explorerModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_explorerModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "explorer module initialized");

    m_sendModule = new SendModule();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    m_sendModule->setObjectName("sendModule");
    connect(m_sendModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_sendModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    this->tabifyDockWidget(m_explorerModule, m_sendModule);
    m_explorerModule->raise();

    m_databaseModule = new DatabaseModule();
    this->addDockWidget(Qt::RightDockWidgetArea, m_databaseModule);
    m_databaseModule->setObjectName("databaseModule");
    connect(m_databaseModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_databaseModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new DatatableModule();
    this->addDockWidget(Qt::RightDockWidgetArea, m_datatableModule);
    m_datatableModule->setObjectName("datatableModule");
    connect(m_datatableModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_datatableModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "datatable module initialized");

    m_dataplotModule = new DataplotModule(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "dataplot module initialized");

    m_logModule = new Log();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_logModule);
    m_logModule->setObjectName("logModule");
    connect(m_logModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_logModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "log module initialized");

    m_diagnosticsModule = new DiagnosticsModule();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_diagnosticsModule);
    m_diagnosticsModule->setObjectName("diagnosticsModule");
    connect(m_diagnosticsModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_diagnosticsModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "diagnostics module initialized");

    m_debugModule = new DebugModule();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_debugModule);
    m_debugModule->setObjectName("debugModule");
    connect(m_debugModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_debugModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "debug module initialized");

    this->tabifyDockWidget(m_logModule, m_diagnosticsModule);
    this->tabifyDockWidget(m_logModule, m_debugModule);
    m_logModule->raise();

    m_threadpoolModule = new ThreadpoolModule();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_threadpoolModule);
    m_threadpoolModule->setObjectName("threadpoolModule");
    connect(m_threadpoolModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_threadpoolModuleView->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "threadpool module initialized");

    m_scriptModule = new ScriptModule();
    this->setCentralWidget(m_scriptModule);

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
    connect(this, &MainWindow::openWorkspace, m_llsModule, &LuaLanguageServer::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_scriptModule, &ScriptModule::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_explorerModule, &ExplorerModule::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_threadpoolModule, &ThreadpoolModule::workspaceOpen);
    connect(m_configModule, &ConfigModule::appendLog, m_logModule, &Log::logAppend);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_scriptModule, &ScriptModule::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_diagnosticsModule, &DiagnosticsModule::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnCompletion, m_scriptModule, &ScriptModule::completionReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFoldingRange, m_scriptModule, &ScriptModule::foldingRangeReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFormatting, m_scriptModule, &ScriptModule::formattingReturn);
    connect(m_llsModule, &LuaLanguageServer::returnHover, m_scriptModule, &ScriptModule::hoverReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSemanticTokens, m_scriptModule, &ScriptModule::semanticTokensReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSignatureHelp, m_scriptModule, &ScriptModule::signatureHelpReturn);
    connect(m_portModule, &PortModule::appendLog, m_logModule, &Log::logAppend);
    connect(m_explorerModule, &ExplorerModule::appendLog, m_logModule, &Log::logAppend);
    connect(m_explorerModule, &ExplorerModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_explorerModule, &ExplorerModule::runScript, m_threadpoolModule, &ThreadpoolModule::threadRun);
    connect(m_explorerModule, &ExplorerModule::debugScript, m_threadpoolModule, &ThreadpoolModule::threadDebug);
    connect(m_datatableModule, &DatatableModule::addGraphDataPlot, m_dataplotModule, &DataplotModule::dataplotAddGraph);
    connect(m_datatableModule, &DatatableModule::addPointDataPlot, m_dataplotModule, &DataplotModule::dataplotAddPoint);
    connect(m_dataplotModule, &DataplotModule::addGraphDatatable, m_datatableModule, &DatatableModule::datatableAddGraph);
    connect(m_diagnosticsModule, &DiagnosticsModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_diagnosticsModule, &DiagnosticsModule::setCursorPosition, m_scriptModule, &ScriptModule::cursorPositionSet);
    connect(m_diagnosticsModule, &DiagnosticsModule::showIndicator, m_scriptModule, &ScriptModule::indicatorShow);
    connect(m_debugModule, &DebugModule::openScript, m_scriptModule, &ScriptModule::scriptOpen);
    connect(m_debugModule, &DebugModule::showMarker, m_scriptModule, &ScriptModule::markerShow);
    connect(m_threadpoolModule, &ThreadpoolModule::startDebug, m_debugModule, &DebugModule::debugStart);
    connect(m_scriptModule, &ScriptModule::requestJson, m_llsModule, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &ScriptModule::notificationJson, m_llsModule, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &ScriptModule::appendLog, m_logModule, &Log::logAppend);
    connect(m_scriptModule, &ScriptModule::openWorkspace, this, &MainWindow::workspaceOpen);
    connect(m_scriptModule, &ScriptModule::insertBreakpoint, m_debugModule, &DebugModule::breakpointInsert);
    connect(m_scriptModule, &ScriptModule::removeBreakpoint, m_debugModule, &DebugModule::breakpointRemove);
    connect(m_scriptModule, &ScriptModule::runThread, m_threadpoolModule, &ThreadpoolModule::threadRun);
    connect(m_scriptModule, &ScriptModule::debugThread, m_threadpoolModule, &ThreadpoolModule::threadDebug);

    g_database = m_databaseModule;
    g_datatable = m_datatableModule;
    g_dataplot = m_dataplotModule;
    g_debug = m_debugModule;
    g_log = m_logModule;
    g_port = m_portModule;
    g_script = m_scriptModule;
    g_threadpool = m_threadpoolModule;
    g_undo = m_undoModule;
}

void MainWindow::workspaceInit() {
    QUrl rootUrl{};
    // check if workspace is valid
    if (rootUrl = QUrl(m_mainConfig["workspace"].toString()); QFileInfo::exists(rootUrl.toLocalFile())) {
        // check if lua config files exist
        const QString rootPath = rootUrl.toLocalFile();
        if (const QString luarcPath = QDir(rootPath).filePath(".luarc.json"); !QFile::exists(luarcPath)) {
            QFile::copy(":/config/.luarc.json", luarcPath);
            QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                             | QFileDevice::ReadUser | QFileDevice::WriteUser
                                             | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json generated");
        } else if (filehashCalc(":/config/.luarc.json") != filehashCalc(luarcPath)) {
            QFile::remove(luarcPath);
            QFile::copy(":/config/.luarc.json", luarcPath);
            QFile::setPermissions(luarcPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                             | QFileDevice::ReadUser | QFileDevice::WriteUser
                                             | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, ".luarc.json updated");
        }
        if (const QString libdPath = QDir(rootPath).filePath("lib.d.lua"); !QFile::exists(libdPath)) {
            QFile::copy(":/config/lib.d.lua", libdPath);
            QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                            | QFileDevice::ReadUser | QFileDevice::WriteUser
                                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua generated");
        } else if (filehashCalc(":/config/lib.d.lua") != filehashCalc(libdPath)) {
            QFile::remove(libdPath);
            QFile::copy(":/config/lib.d.lua", libdPath);
            QFile::setPermissions(libdPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                            | QFileDevice::ReadUser | QFileDevice::WriteUser
                                            | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "lib.d.lua updated");
        }
    }
    emit openWorkspace(rootUrl);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace loaded");
}

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_config["shortcutConfig"].toObject();
    m_openWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["openWorkspace"].toString()), this); // NOLINT
    connect(m_openWorkspaceShortcut, &QShortcut::activated, this, [this] {
        workspaceOpen();
    });
    m_saveWorkspaceShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspace"].toString()), this); // NOLINT
    connect(m_saveWorkspaceShortcut, &QShortcut::activated, this, [this] {
        workspaceSave();
    });
    m_saveWorkspaceAsShortcut = new QShortcut(QKeySequence(shortcutConfig["saveWorkspaceAs"].toString()), this); // NOLINT
    connect(m_saveWorkspaceAsShortcut, &QShortcut::activated, this, [this] {
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

void MainWindow::menuInit() {
    auto *menuBar = new QMenuBar(); // NOLINT
    setMenuBar(menuBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        menuBar->addMenu(fileMenu);
        auto shortcutConfig = g_config["shortcutConfig"].toObject();
        auto *openWorkspaceAction = new QAction(tr("Open Workspace") + "\t" + shortcutConfig["openWorkspace"].toString()); // NOLINT
        fileMenu->addAction(openWorkspaceAction);
        auto *saveWorkspaceAction = new QAction(tr("Save Workspace") + "\t" + shortcutConfig["saveWorkspace"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAction);
        auto *saveWorkspaceAsAction = new QAction(tr("Save Workspace As") + "\t" + shortcutConfig["saveWorkspaceAs"].toString()); // NOLINT
        fileMenu->addAction(saveWorkspaceAsAction);
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
        menuBar->addMenu(viewMenu);
        m_portModuleView = new QAction(tr("port"));
        viewMenu->addAction(m_portModuleView);
        m_portModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_portModuleView->setChecked(m_portModule->isVisible()); });
        connect(m_portModuleView, &QAction::triggered, this, [this](const bool visible) { m_portModule->setVisible(visible); });
        m_explorerModuleView = new QAction(tr("explorer"));
        viewMenu->addAction(m_explorerModuleView);
        m_explorerModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_explorerModuleView->setChecked(m_explorerModule->isVisible()); });
        connect(m_explorerModuleView, &QAction::triggered, this, [this](const bool visible) { m_explorerModule->setVisible(visible); });
        m_sendModuleView = new QAction(tr("send"));
        viewMenu->addAction(m_sendModuleView);
        m_sendModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_sendModuleView->setChecked(m_sendModule->isVisible()); });
        connect(m_sendModuleView, &QAction::triggered, this, [this](const bool visible) { m_sendModule->setVisible(visible); });
        m_databaseModuleView = new QAction(tr("database"));
        viewMenu->addAction(m_databaseModuleView);
        m_databaseModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_databaseModuleView->setChecked(m_databaseModule->isVisible()); });
        connect(m_databaseModuleView, &QAction::triggered, this, [this](const bool visible) { m_databaseModule->setVisible(visible); });
        m_datatableModuleView = new QAction(tr("data table"));
        viewMenu->addAction(m_datatableModuleView);
        m_datatableModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_datatableModuleView->setChecked(m_datatableModule->isVisible()); });
        connect(m_datatableModuleView, &QAction::triggered, this, [this](const bool visible) { m_datatableModule->setVisible(visible); });
        m_dataplotModuleView = new QAction(tr("data plot"));
        viewMenu->addAction(m_dataplotModuleView);
        m_dataplotModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_dataplotModuleView->setChecked(m_dataplotModule->isVisible()); });
        connect(m_dataplotModuleView, &QAction::triggered, this, [this](const bool visible) { m_dataplotModule->setVisible(visible); });
        m_logModuleView = new QAction(tr("log"));
        viewMenu->addAction(m_logModuleView);
        m_logModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_logModuleView->setChecked(m_logModule->isVisible()); });
        connect(m_logModuleView, &QAction::triggered, this, [this](const bool visible) { m_logModule->setVisible(visible); });
        m_diagnosticsModuleView = new QAction(tr("diagnostics"));
        viewMenu->addAction(m_diagnosticsModuleView);
        m_diagnosticsModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_diagnosticsModuleView->setChecked(m_diagnosticsModule->isVisible()); });
        connect(m_diagnosticsModuleView, &QAction::triggered, this, [this](const bool visible) { m_diagnosticsModule->setVisible(visible); });
        m_debugModuleView = new QAction(tr("debug"));
        viewMenu->addAction(m_debugModuleView);
        m_debugModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_debugModuleView->setChecked(m_debugModule->isVisible()); });
        connect(m_debugModuleView, &QAction::triggered, this, [this](const bool visible) { m_debugModule->setVisible(visible); });
        m_threadpoolModuleView = new QAction(tr("threadpool"));
        viewMenu->addAction(m_threadpoolModuleView);
        m_threadpoolModuleView->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_threadpoolModuleView->setChecked(m_threadpoolModule->isVisible()); });
        connect(m_threadpoolModuleView, &QAction::triggered, this, [this](const bool visible) { m_threadpoolModule->setVisible(visible); });
    }
}

void MainWindow::layoutInit() {
    if (!m_mainConfig["geometry"].isString()) return;
    const QByteArray geometry = QByteArray::fromBase64(m_mainConfig["geometry"].toString().toLatin1());
    restoreGeometry(geometry);
    if (!m_mainConfig["state"].isString()) return;
    const QByteArray state = QByteArray::fromBase64(m_mainConfig["state"].toString().toLatin1());
    restoreState(state);
}

void MainWindow::mainConfigSave() {
    m_mainConfig["geometry"] = QString(saveGeometry().toBase64());
    m_mainConfig["state"] = QString(saveState().toBase64());
    g_config["mainConfig"] = m_mainConfig;
}

void MainWindow::workspaceSave(const QString &filePath) {
    m_scriptModule->scriptConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    mainConfigSave();
    m_configModule->configSave(filePath);
}
