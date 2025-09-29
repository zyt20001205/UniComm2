#include "../include/mainWindow.h"

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
    const QUrl rootUrl(g_config["mainConfig"].toObject()["workspace"].toString());
    workspaceInit(rootUrl);
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

void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_llsModule = new LuaLanguageServer(this);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "lls module initialized");

    m_scriptModule = new Script();
    this->setCentralWidget(m_scriptModule);

    m_portModule = new Port();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_portModule);
    m_portModule->setObjectName("portModule");
    connect(m_portModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) {
        m_viewPort->setChecked(visible);
    });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_sendModule = new Send();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    m_sendModule->setObjectName("sendModule");
    connect(m_sendModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) {
        m_viewSend->setChecked(visible);
    });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    m_databaseModule = new Database();
    this->addDockWidget(Qt::RightDockWidgetArea, m_databaseModule);
    m_databaseModule->setObjectName("databaseModule");
    connect(m_databaseModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) {
        m_viewDatabase->setChecked(visible);
    });
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new Datatable();
    this->addDockWidget(Qt::RightDockWidgetArea, m_datatableModule);
    m_datatableModule->setObjectName("datatableModule");
    connect(m_datatableModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) {
        m_viewDatatable->setChecked(visible);
    });
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
    connect(m_logModule, &QDockWidget::visibilityChanged, this, [this](const bool visible) {
        m_viewLog->setChecked(visible);
    });

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
    connect(this, &MainWindow::loadWorkspace, m_llsModule, &LuaLanguageServer::workspaceLoad);
    connect(this, &MainWindow::loadWorkspace, m_scriptModule, &Script::workspaceLoad);
    connect(this, &MainWindow::loadWorkspace, m_scriptModule->m_scriptExplorerTreeView, &ScriptExplorer::workspaceLoad);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_scriptModule, &Script::diagnosticsReturn);
    connect(m_llsModule, &LuaLanguageServer::returnCompletion, m_scriptModule, &Script::completionReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFoldingRange, m_scriptModule, &Script::foldingRangeReturn);
    connect(m_llsModule, &LuaLanguageServer::returnFormatting, m_scriptModule, &Script::formattingReturn);
    connect(m_llsModule, &LuaLanguageServer::returnHover, m_scriptModule, &Script::hoverReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSemanticTokens, m_scriptModule, &Script::semanticTokensReturn);
    connect(m_llsModule, &LuaLanguageServer::returnSignatureHelp, m_scriptModule, &Script::signatureHelpReturn);
    connect(m_portModule, &Port::appendLog, m_logModule, &Log::logAppend);
    connect(m_scriptModule, &Script::appendLog, m_logModule, &Log::logAppend);
    connect(m_scriptModule, &Script::requestJson, m_llsModule, &LuaLanguageServer::jsonRequest);
    connect(m_scriptModule, &Script::notificationJson, m_llsModule, &LuaLanguageServer::jsonNotification);
    connect(m_datatableModule, &Datatable::addGraphDataPlot, m_dataplotModule, &Dataplot::dataplotAddGraph);
    connect(m_datatableModule, &Datatable::addPointDataPlot, m_dataplotModule, &Dataplot::dataplotAddPoint);
    connect(m_dataplotModule, &Dataplot::addGraphDatatable, m_datatableModule, &Datatable::datatableAddGraph);

    m_sendModule->setPort(m_portModule);
    g_script = m_scriptModule;
    g_database = m_databaseModule;
    g_datatable = m_datatableModule;
    g_dataplot = m_dataplotModule;
    g_log = m_logModule;
    g_port = m_portModule;
}

void MainWindow::workspaceInit(const QUrl &rootUrl) {
    QUrl url = rootUrl;
    if (url.isEmpty()) {
        const QString rootDir = QFileDialog::getExistingDirectory(
            this,
            tr("Select Workspace"),
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        url = QUrl::fromLocalFile(rootDir);
    }
    // check if workspace is valid
    if (url.isLocalFile()) {
        // generate lua config files
        const QString rootPath = url.toLocalFile();
        if (const QString luarcPath = QDir(rootPath).filePath(".luarc.json"); !QFile::exists(luarcPath)) {
            QFile::copy(":/config/.luarc.json", luarcPath);
        }
        if (const QString libdPath = QDir(rootPath).filePath("lib.d.lua"); !QFile::exists(libdPath)) {
            QFile::copy(":/config/lib.d.lua", libdPath);
        }
        // load workspace
        m_mainConfig["workspace"] = url.toString();
        emit loadWorkspace(url);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "workspace loaded");
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "workspace not found");
    }
}

void MainWindow::menuInit() {
    auto *menuBar = new QMenuBar(); // NOLINT
    setMenuBar(menuBar);
    // file menu
    {
        auto *fileMenu = new QMenu(tr("File")); // NOLINT
        menuBar->addMenu(fileMenu);
        auto *loadWorkspaceAction = new QAction(tr("Load Workspace")); // NOLINT
        fileMenu->addAction(loadWorkspaceAction);
        connect(loadWorkspaceAction, &QAction::triggered, this, [this] {
            workspaceInit(QUrl());
        });
    }
    // view menu
    {
        auto *viewMenu = new QMenu(tr("View")); // NOLINT
        menuBar->addMenu(viewMenu);
        m_viewPort = new QAction(tr("port"));
        viewMenu->addAction(m_viewPort);
        m_viewPort->setCheckable(true);
        QTimer::singleShot(0, this, [this] { m_viewPort->setChecked(m_portModule->isVisible()); });
        connect(m_viewPort, &QAction::triggered, this, [this](const bool visible) { m_portModule->setVisible(visible); });
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
