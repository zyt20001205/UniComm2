#include "../include/init.h"

void Init::configInit() {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing config");
    Config config;
    config.configInit();
}

void Init::moduleInit(QMainWindow *mainWindow) {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "initializing module");

    const auto scriptModule = new Script(mainWindow);
    mainWindow->setCentralWidget(scriptModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script module initialized");

    const auto portModule = new Port(mainWindow);
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, portModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "port module initialized");

    const auto sendModule = new Send(mainWindow);
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, sendModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "send module initialized");

    const auto logModule = new Log(mainWindow);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, logModule);
    // logging
    timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "log module initialized");


    QObject::connect(portModule, &Port::appendLog, logModule, &Log::logAppend);
    QObject::connect(sendModule, &Send::writePort, portModule, &Port::portWrite);
    QObject::connect(scriptModule, &Script::openPort, portModule, &Port::portOpen);
    QObject::connect(scriptModule, &Script::closePort, portModule, &Port::portClose);
    QObject::connect(scriptModule, &Script::writePort, portModule, &Port::portWrite);
    QObject::connect(scriptModule, &Script::appendLog, logModule, &Log::logAppend);

    scriptModule->setPort(portModule);
}
