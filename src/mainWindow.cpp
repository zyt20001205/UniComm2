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

    // config init
    configInit();
    // module init
    moduleInit();
    // menu init
    menuInit();
    // shortcut init
    shortcutInit();
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
}

void MainWindow::moduleInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    m_llsModule = new LuaLanguageServer(this);

    m_scriptModule = new Script();
    this->setCentralWidget(m_scriptModule);

    m_portModule = new Port();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_portModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_sendModule = new Send();
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    m_databaseModule = new Database();
    this->addDockWidget(Qt::RightDockWidgetArea, m_databaseModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "database module initialized");

    m_datatableModule = new Datatable();
    this->addDockWidget(Qt::RightDockWidgetArea, m_datatableModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "datatable module initialized");

    m_logModule = new Log();
    this->addDockWidget(Qt::BottomDockWidgetArea, m_logModule);

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
    connect(m_llsModule, &LuaLanguageServer::returnPublishDiagnostics, m_scriptModule, &Script::diagnosticsReturn);
    connect(m_llsModule,&LuaLanguageServer::returnFormatting, m_scriptModule, &Script::formattingReturn);
    connect(m_llsModule,&LuaLanguageServer::returnHover, m_scriptModule, &Script::hoverReturn);
    connect(m_llsModule,&LuaLanguageServer::returnSemanticTokens, m_scriptModule, &Script::semanticTokensReturn);
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
}

void MainWindow::shortcutInit() {
    auto shortcutConfig = g_config["shortcutConfig"].toObject();
    auto shortcutSave = new QShortcut(QKeySequence(shortcutConfig["save"].toString()), this); // NOLINT
    connect(shortcutSave, &QShortcut::activated, this, [this] {
        saveConfig();
        emit appendLog("workspace saved", "info");
    });
}

void MainWindow::saveConfig() const {
    m_scriptModule->scriptConfigSave();
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_databaseModule->databaseConfigSave();
    m_datatableModule->datatableConfigSave();
    m_logModule->logConfigSave();
    m_configModule->configSave();
}
