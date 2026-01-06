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
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
        }
        QTabBar::tab {
            background: transparent;
            border: none;
            padding: 6px 12px;
            color: #606060;
        }
        QTabBar::tab:hover {
            border-bottom: 2px solid #cccccc;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            color: #000000;
            border-bottom: 2px solid #0f548c;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
    )");
    QQuickStyle::setStyle("FluentWinUI3");
    // kddw init
    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    flags |= KDDockWidgets::Config::Flag_AllowReorderTabs;
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    KDDockWidgets::Config::self().setFlags(flags);
    KDDockWidgets::Config::self().setSeparatorThickness(3);
    KDDockWidgets::Config::self().setViewFactory(new CustomWidgetFactory());
    // config init
    if (ConfigManager::mainConfigLoad()) return 1;
    auto *mainWindow = new MainWindow();
    mainWindow->show();
    // application exec
    return QApplication::exec();
}
