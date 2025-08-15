#include <QApplication>
#include <QStyleFactory>
#include "../include/mainWindow.h"
#include "../include/init.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    MainWindow mainWindow;

    Init::configInit();
    Init::moduleInit(&mainWindow);

    mainWindow.show();

    return QApplication::exec();
}