#include "../include/mainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    uiInit();
}

void MainWindow::uiInit() {
    setWindowTitle("UniComm");
    resize(1280, 720);

    setDockNestingEnabled(true);

    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks);

    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "main window created");
}
