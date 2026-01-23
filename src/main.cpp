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

        QScrollBar:horizontal {
            background-color: transparent;
            height: 10px;
            margin: 0px 0px 0px 0px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: #b3b3b3;
            border-radius: 3px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:pressed {
            background-color: #e0e0e0;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0px;
            background: none;
        }

        QScrollBar:vertical {
            background-color: transparent;
            width: 10px;
            margin: 0px 0px 0px 0px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: #b3b3b3;
            border-radius: 3px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:pressed {
            background-color: #e0e0e0;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
            background: none;
        }

        QAbstractScrollArea::corner {
             background: #ffffff;
         }

        QTabBar::tab {
            background: transparent;
            border: none;
            color: #606060;
            font-family: "Segoe UI";
            font-size: 14px;
            margin: 10px 10px 6px 10px;
            padding: 0px 0px 4px 16px;
        }
        QTabBar::tab:hover {
            border-bottom: 3px solid #ebebeb;
        }
        QTabBar::tab:selected {
            border-bottom: 3px solid #0f548c;
            color: #000000;
            font-weight: 600;
        }
        QTabBar::close-button {
            border-radius: 4px;
            margin: 2px;
            image: url(:/icon/close.svg);
        }
        QTabBar::close-button:hover {
            background-color: #ebebeb;
        }
        QTabBar::close-button:pressed {
            background-color: #d6d6d6;
        }
        QTabWidget::pane {
            border: 1px solid #cccccc;
            border-radius: 2px;
            border-top: none;
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
