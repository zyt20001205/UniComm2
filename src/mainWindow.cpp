#include "mainWindow.h"

#include "config.h"
#include "database.h"
#include "dataplot.h"
#include "datatableModule.h"
#include "debug.h"
#include "diagnostics.h"
#include "explorer.h"
#include "globals.h"
#include "log.h"
#include "luaLanguageServer.h"
#include "portModule/portModule.h"
#include "script.h"
#include "SendModule.h"
#include "threadpool.h"
#include "undoModule.h"
#include "utils.h"

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
    workspaceInit();
    moduleInit();
    menuInit();
    shortcutInit();
    layoutInit();
}

// MainWindow protected
void MainWindow::closeEvent(QCloseEvent *event) {
    const QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Exit", "Save and exit?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        saveConfig();
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
    m_configModule = new Config;
    m_configModule->configInit();
    m_mainConfig = g_config["mainConfig"].toObject();
}

void MainWindow::workspaceInit() {
    // check if workspace is valid
    if (const QUrl rootUrl(m_mainConfig["workspace"].toString()); !rootUrl.isEmpty() && rootUrl.isLocalFile()) {
        // examine lua config files
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
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "workspace loaded");
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "workspace not found");
    }
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
    connect(m_portModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewPort->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_explorerModule = new Explorer();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_explorerModule);
    m_explorerModule->setObjectName("explorerModule");
    connect(m_explorerModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewExplorer->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "explorer module initialized");

    m_sendModule = new SendModule();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    m_sendModule->setObjectName("sendModule");
    connect(m_sendModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewSend->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    this->tabifyDockWidget(m_explorerModule, m_sendModule);
    m_explorerModule->raise();

    m_databaseModule = new Database();
    this->addDockWidget(Qt::RightDockWidgetArea, m_databaseModule);
    m_databaseModule->setObjectName("databaseModule");
    connect(m_databaseModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewDatabase->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new DatatableModule();
    this->addDockWidget(Qt::RightDockWidgetArea, m_datatableModule);
    m_datatableModule->setObjectName("datatableModule");
    connect(m_datatableModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewDatatable->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "datatable module initialized");

    m_dataplotModule = new Dataplot(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "dataplot module initialized");

    m_logModule = new Log();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_logModule);
    m_logModule->setObjectName("logModule");
    connect(m_logModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewLog->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "log module initialized");

    m_diagnosticsModule = new Diagnostics();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_diagnosticsModule);
    m_diagnosticsModule->setObjectName("diagnosticsModule");
    connect(m_diagnosticsModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewDiagnostics->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "diagnostics module initialized");

    m_debugModule = new Debug();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_debugModule);
    m_debugModule->setObjectName("debugModule");
    connect(m_debugModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewDebug->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "debug module initialized");

    this->tabifyDockWidget(m_logModule, m_diagnosticsModule);
    this->tabifyDockWidget(m_logModule, m_debugModule);
    m_logModule->raise();

    m_threadpoolModule = new Threadpool();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_threadpoolModule);
    m_threadpoolModule->setObjectName("threadpoolModule");
    connect(m_threadpoolModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) { m_viewThreadpool->setChecked(visible); });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "threadpool module initialized");

    m_scriptModule = new Script();
    this->setCentralWidget(m_scriptModule);

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
    connect(this, &MainWindow::openWorkspace, m_llsModule, &LuaLanguageServer::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_scriptModule, &Script::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_explorerModule, &Explorer::workspaceOpen);
    connect(this, &MainWindow::openWorkspace, m_threadpoolModule, &Threadpool::workspaceOpen);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_scriptModule, &Script::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_diagnosticsModule, &Diagnostics::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnCompletion, m_scriptModule, &Script::completionReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFoldingRange, m_scriptModule, &Script::foldingRangeReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFormatting, m_scriptModule, &Script::formattingReturn);
    connect(m_llsModule, &LuaLanguageServer::returnHover, m_scriptModule, &Script::hoverReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSemanticTokens, m_scriptModule, &Script::semanticTokensReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSignatureHelp, m_scriptModule, &Script::signatureHelpReturn);
    connect(m_portModule, &PortModule::appendLog, m_logModule, &Log::logAppend);
    connect(m_explorerModule, &Explorer::appendLog, m_logModule, &Log::logAppend);
    connect(m_explorerModule, &Explorer::openScript, m_scriptModule, &Script::scriptOpen);
    connect(m_explorerModule, &Explorer::runScript, m_threadpoolModule, &Threadpool::threadRun);
    connect(m_explorerModule, &Explorer::debugScript, m_threadpoolModule, &Threadpool::threadDebug);
    connect(m_datatableModule, &DatatableModule::addGraphDataPlot, m_dataplotModule, &Dataplot::dataplotAddGraph);
    connect(m_datatableModule, &DatatableModule::addPointDataPlot, m_dataplotModule, &Dataplot::dataplotAddPoint);
    connect(m_dataplotModule, &Dataplot::addGraphDatatable, m_datatableModule, &DatatableModule::datatableAddGraph);
    connect(m_diagnosticsModule, &Diagnostics::openScript, m_scriptModule, &Script::scriptOpen);
    connect(m_diagnosticsModule, &Diagnostics::setCursorPosition, m_scriptModule, &Script::cursorPositionSet);
    connect(m_diagnosticsModule, &Diagnostics::showIndicator, m_scriptModule, &Script::indicatorShow);
    connect(m_debugModule, &Debug::openScript, m_scriptModule, &Script::scriptOpen);
    connect(m_debugModule, &Debug::showMarker, m_scriptModule, &Script::markerShow);
    connect(m_threadpoolModule, &Threadpool::startDebug, m_debugModule, &Debug::debugStart);
    connect(m_scriptModule, &Script::requestJson, m_llsModule, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &Script::notificationJson, m_llsModule, &LuaLanguageServer::jsonNotification);
    connect(m_scriptModule, &Script::appendLog, m_logModule, &Log::logAppend);
    connect(m_scriptModule, &Script::openWorkspace, this, &MainWindow::workspaceInit);
    connect(m_scriptModule, &Script::insertBreakpoint, m_debugModule, &Debug::breakpointInsert);
    connect(m_scriptModule, &Script::removeBreakpoint, m_debugModule, &Debug::breakpointRemove);
    connect(m_scriptModule, &Script::runThread, m_threadpoolModule, &Threadpool::threadRun);
    connect(m_scriptModule, &Script::debugThread, m_threadpoolModule, &Threadpool::threadDebug);

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

void MainWindow::menuInit() {
    auto *menuBar = new QMenuBar(); // NOLINT
    setMenuBar(menuBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        menuBar->addMenu(fileMenu);
        auto *openWorkspaceAction = new QAction(tr("Open Workspace")); // NOLINT
        fileMenu->addAction(openWorkspaceAction);
        connect(openWorkspaceAction, &QAction::triggered, this, [this] {
            const QString rootDir = QFileDialog::getExistingDirectory(
                this,
                tr("Select Workspace"),
                QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
            );
            const QUrl url = QUrl::fromLocalFile(rootDir);
            m_mainConfig["workspace"] = url.toString();
            workspaceInit();
            emit openWorkspace(url);
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
        menuBar->addMenu(viewMenu);
        m_viewPort = new QAction(tr("port"));
        viewMenu->addAction(m_viewPort);
        m_viewPort->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewPort->setChecked(m_portModule->isVisible()); });
        connect(m_viewPort, &QAction::triggered, this, [this](const bool visible) { m_portModule->setVisible(visible); });
        m_viewExplorer = new QAction(tr("explorer"));
        viewMenu->addAction(m_viewExplorer);
        m_viewExplorer->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewExplorer->setChecked(m_explorerModule->isVisible()); });
        connect(m_viewExplorer, &QAction::triggered, this, [this](const bool visible) { m_explorerModule->setVisible(visible); });
        m_viewSend = new QAction(tr("send"));
        viewMenu->addAction(m_viewSend);
        m_viewSend->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewSend->setChecked(m_sendModule->isVisible()); });
        connect(m_viewSend, &QAction::triggered, this, [this](const bool visible) { m_sendModule->setVisible(visible); });
        m_viewDatabase = new QAction(tr("database"));
        viewMenu->addAction(m_viewDatabase);
        m_viewDatabase->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewDatabase->setChecked(m_databaseModule->isVisible()); });
        connect(m_viewDatabase, &QAction::triggered, this, [this](const bool visible) { m_databaseModule->setVisible(visible); });
        m_viewDatatable = new QAction(tr("data table"));
        viewMenu->addAction(m_viewDatatable);
        m_viewDatatable->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewDatatable->setChecked(m_datatableModule->isVisible()); });
        connect(m_viewDatatable, &QAction::triggered, this, [this](const bool visible) { m_datatableModule->setVisible(visible); });
        m_viewDataplot = new QAction(tr("data plot"));
        viewMenu->addAction(m_viewDataplot);
        m_viewDataplot->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewDataplot->setChecked(m_dataplotModule->isVisible()); });
        connect(m_viewDataplot, &QAction::triggered, this, [this](const bool visible) { m_dataplotModule->setVisible(visible); });
        m_viewLog = new QAction(tr("log"));
        viewMenu->addAction(m_viewLog);
        m_viewLog->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewLog->setChecked(m_logModule->isVisible()); });
        connect(m_viewLog, &QAction::triggered, this, [this](const bool visible) { m_logModule->setVisible(visible); });
        m_viewDiagnostics = new QAction(tr("diagnostics"));
        viewMenu->addAction(m_viewDiagnostics);
        m_viewDiagnostics->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewDiagnostics->setChecked(m_diagnosticsModule->isVisible()); });
        connect(m_viewDiagnostics, &QAction::triggered, this, [this](const bool visible) { m_diagnosticsModule->setVisible(visible); });
        m_viewDebug = new QAction(tr("debug"));
        viewMenu->addAction(m_viewDebug);
        m_viewDebug->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewDebug->setChecked(m_debugModule->isVisible()); });
        connect(m_viewDebug, &QAction::triggered, this, [this](const bool visible) { m_debugModule->setVisible(visible); });
        m_viewThreadpool = new QAction(tr("threadpool"));
        viewMenu->addAction(m_viewThreadpool);
        m_viewThreadpool->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewThreadpool->setChecked(m_threadpoolModule->isVisible()); });
        connect(m_viewThreadpool, &QAction::triggered, this, [this](const bool visible) { m_threadpoolModule->setVisible(visible); });
    }
}

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_config["shortcutConfig"].toObject();
    auto shortcutSave = new QShortcut(QKeySequence(shortcutConfig["save"].toString()), this); // NOLINT
    connect(shortcutSave, &QShortcut::activated, this, [this] {
        saveConfig();
        emit appendLog("workspace saved", "info");
    });
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

void MainWindow::saveConfig() {
    m_scriptModule->scriptConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    mainConfigSave();
    m_configModule->configSave();
}
