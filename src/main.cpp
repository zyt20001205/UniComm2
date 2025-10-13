#include <QStyleFactory>
#include <kddockwidgets/Config.h>

#include "mainWindow.h"
#include "globals.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    KDDockWidgets::Config::self().setFlags(flags);

    auto *mainWindow = new MainWindow();
    mainWindow->show();
    g_mainWindow = mainWindow;

    return QApplication::exec();
}
