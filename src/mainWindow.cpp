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

    m_scriptModule = new Script();
    this->setCentralWidget(m_scriptModule);

    m_portModule = new Port();
    m_portModule->setObjectName("portModule");
    this->addDockWidget(Qt::LeftDockWidgetArea, m_portModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_sendModule = new Send();
    m_sendModule->setObjectName("sendModule");
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    m_databaseModule = new Database();
    m_databaseModule->setObjectName("databaseModule");
    this->addDockWidget(Qt::RightDockWidgetArea, m_databaseModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new Datatable();
    m_datatableModule->setObjectName("datatableModule");
    this->addDockWidget(Qt::RightDockWidgetArea, m_datatableModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "datatable module initialized");

    m_logModule = new Log();
    m_logModule->setObjectName("logModule");
    this->addDockWidget(Qt::BottomDockWidgetArea, m_logModule);

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
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

    m_sendModule->setPort(m_portModule);
    g_database = m_databaseModule;
    g_datatable = m_datatableModule;
    g_log = m_logModule;
    g_port = m_portModule;
}

void MainWindow::menuInit() {
    auto *menuBar = new QMenuBar(); // NOLINT
    setMenuBar(menuBar);

    auto *viewMenu = new QMenu(tr("view")); // NOLINT
    menuBar->addMenu(viewMenu);
    m_viewPort = new QAction(tr("port")); // NOLINT
    viewMenu->addAction(m_viewPort);
    m_viewPort->setCheckable(true);
    QTimer::singleShot(0, this, [this] {
        m_viewPort->setChecked(m_portModule->isVisible());
    });
    connect(m_viewPort,&QAction::triggered,this,[this](const bool visible) {
       m_portModule->setVisible(visible);
    });
    m_viewSend = new QAction(tr("send")); // NOLINT
    viewMenu->addAction(m_viewSend);
    m_viewSend->setCheckable(true);
    QTimer::singleShot(0, this, [this] {
        m_viewSend->setChecked(m_sendModule->isVisible());
    });
    connect(m_viewSend,&QAction::triggered,this,[this](const bool visible) {
       m_sendModule->setVisible(visible);
    });
    m_viewDatabase = new QAction(tr("database")); // NOLINT
    viewMenu->addAction(m_viewDatabase);
    m_viewDatabase->setCheckable(true);
    QTimer::singleShot(0, this, [this] {
        m_viewDatabase->setChecked(m_databaseModule->isVisible());
    });
    connect(m_viewDatabase,&QAction::triggered,this,[this](const bool visible) {
       m_databaseModule->setVisible(visible);
    });
    m_viewDatatable = new QAction(tr("data table")); // NOLINT
    viewMenu->addAction(m_viewDatatable);
    m_viewDatatable->setCheckable(true);
    QTimer::singleShot(0, this, [this] {
        m_viewDatatable->setChecked(m_datatableModule->isVisible());
    });
    connect(m_viewDatatable,&QAction::triggered,this,[this](const bool visible) {
       m_datatableModule->setVisible(visible);
    });
    m_viewLog = new QAction(tr("log")); // NOLINT
    viewMenu->addAction(m_viewLog);
    m_viewLog->setCheckable(true);
    QTimer::singleShot(0, this, [this] {
        m_viewLog->setChecked(m_logModule->isVisible());
    });
    connect(m_viewLog,&QAction::triggered,this,[this](const bool visible) {
       m_logModule->setVisible(visible);
    });
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

void MainWindow::layoutSave() {
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
    layoutSave();
    m_configModule->configSave();
}
