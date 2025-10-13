#include <QApplication>
#include <QStyleFactory>

#include "mainWindow.h"
#include "globals.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    auto *mainWindow = new MainWindow();
    mainWindow->show();
    g_mainWindow = mainWindow;

    return QApplication::exec();
}