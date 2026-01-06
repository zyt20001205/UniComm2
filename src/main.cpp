#include "mainWindow/mainWindow.h"

#include <crashHandler.h>
#include <QQuickStyle>
#include <QStyleFactory>
#include <kddockwidgets/Config.h>

#include "configManager.h"
#include "mainWindow/kddwCustom.h"

int main(int argc, char *argv[]) {
    // crash handler init
    CrashHandler::init();
    // application style init
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QQuickStyle::setStyle("FluentWinUI3");
    // kddw init
    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    KDDockWidgets::Config::self().setFlags(flags);
    KDDockWidgets::Config::self().setViewFactory(new CustomWidgetFactory());
    // config init
    if (ConfigManager::mainConfigLoad()) return 1;
    auto *mainWindow = new MainWindow();
    mainWindow->show();
    // application exec
    return QApplication::exec();
}
