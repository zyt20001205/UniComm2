#include "mainWindow/mainWindow.h"

#include <core/crashHandler.h>
#include <kddockwidgets/Config.h>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStyleFactory>

#include "core/configManager.h"
#include "mainWindow/kddwCustom.h"

int main(int argc, char *argv[]) {
    // crash handler init
    CrashHandler::init();
    // application style init
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/icon/icon.ico"));
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QQuickStyle::setStyle("FluentWinUI3");
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
    // kddw init
    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    flags |= KDDockWidgets::Config::Flag_AllowReorderTabs;
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    KDDockWidgets::Config::self().setFlags(flags);
    KDDockWidgets::Config::self().setSeparatorThickness(5);
    // config init
    if (ConfigManager::mainConfigLoad()) return 1;
    KDDockWidgets::Config::self().setViewFactory(new CustomWidgetFactory());
    KDDockWidgets::Config::self().setDockWidgetFactoryFunc(dockWidgetFactory);
    auto *mainWindow = new MainWindow();
    mainWindow->show();
    // application exec
    return QApplication::exec();
}
