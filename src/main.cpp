#include "mainWindow.h"

#include <QStyleFactory>
#include <kddockwidgets/Config.h>

#include "configModule.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    KDDockWidgets::Config::self().setFlags(flags);

    if (ConfigModule::mainConfigLoad()) return 1;
    auto *mainWindow = new MainWindow();
    mainWindow->show();

    return QApplication::exec();
}
