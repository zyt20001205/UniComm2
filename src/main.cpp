#include "../include/mainWindow.h"

#include "../include/globals.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    auto *mainWindow = new MainWindow();
    mainWindow->show();
    g_mainWindow = mainWindow;

    return app.exec();
}
