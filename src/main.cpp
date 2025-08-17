#include <QApplication>
#include <QStyleFactory>
#include "../include/mainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}