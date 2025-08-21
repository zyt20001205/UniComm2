#include "../include/mainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    init();
}

void MainWindow::init() {
    // mainWindow ui init
    setWindowTitle("UniComm");
    resize(1280, 720);
    setDockNestingEnabled(true);
    setDockOptions(AllowNestedDocks | AllowTabbedDocks | AnimatedDocks);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main window created");

    // config init
    configInit();
    // module init
    moduleInit();
    // shortcut init
    shortcutInit();
}

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

    m_scriptModule = new Script(this);
    this->setCentralWidget(m_scriptModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    m_portModule = new Port(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, m_portModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    m_sendModule = new Send(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, m_sendModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    m_explorerModule = new Explorer(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, m_explorerModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "explorer module initialized");

    m_logModule = new Log(this);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_logModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "log module initialized");

    connect(this, &MainWindow::appendLog, m_logModule, &Log::logAppend);
    connect(m_portModule, &Port::appendLog, m_logModule, &Log::logAppend);
    connect(m_sendModule, &Send::writePort, m_portModule, &Port::portWrite);
    connect(m_explorerModule, &Explorer::loadScript, m_scriptModule, &Script::scriptLoad);
    connect(m_explorerModule, &Explorer::appendLog, m_logModule, &Log::logAppend);
    connect(m_scriptModule, &Script::openPort, m_portModule, &Port::portOpen);
    connect(m_scriptModule, &Script::closePort, m_portModule, &Port::portClose);
    connect(m_scriptModule, &Script::writePort, m_portModule, &Port::portWrite);
    connect(m_scriptModule, &Script::appendLog, m_logModule, &Log::logAppend);

    m_scriptModule->setPort(m_portModule);
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
    m_portModule->portConfigSave();
    m_sendModule->sendConfigSave();
    m_scriptModule->scriptConfigSave();
    m_logModule->logConfigSave();
    m_configModule->configSave();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Exit", "Save and exit?",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes) {
        saveConfig();
        event->accept();
    } else {
        event->ignore();
    }
}
