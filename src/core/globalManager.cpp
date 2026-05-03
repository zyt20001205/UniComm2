#include "core/globalManager.h"

#include <QStyleHints>

#include "kddockwidgets/Config.h"
#include "mainWindow/kddwCustom.h"

// public
GlobalManager::GlobalManager(QWidget *parent)
    : QObject(parent),
      m_palette({
          {"lightFore", "#242424"},
          {"darkFore", "#ffffff"},
          {"lightForeHover", "#242424"},
          {"darkForeHover", "#ffffff"},
          {"lightForePressed", "#242424"},
          {"darkForePressed", "#ffffff"},
          {"lightForeSelected", "#242424"},
          {"darkForeSelected", "#ffffff"},

          {"lightBack", "#ffffff"},
          {"darkBack", "#292929"},
          {"lightBackHover", "#f5f5f5"},
          {"darkBackHover", "#3d3d3d"},
          {"lightBackPressed", "#e0e0e0"},
          {"darkBackPressed", "#1f1f1f"},
          {"lightBackSelected", "#ebebeb"},
          {"darkBackSelected", "#383838"},

          {"lightBrandBack", "#0f6cbd"},
          {"darkBrandBack", "#115ea3"},

          {"lightSuccessFore", "#107c10"},
          {"darkSuccessFore", "#9fd89f"},
          {"lightSuccessBack", "#107c10"},
          {"darkSuccessBack", "#107c10"},

          {"lightWarningFore", "#bc4b09"},
          {"darkWarningFore", "#f98845"},
          {"lightWarningBack", "#f7630c"},
          {"darkWarningBack", "#f7630c"},

          {"lightDangerFore", "#c50f1f"},
          {"darkDangerFore", "#eeacb2"},
          {"lightDangerBack", "#c50f1f"},
          {"darkDangerBack", "#c50f1f"},
      }),
      m_styleSheet({
          // Light
          "QMainWindow { background-color: " + m_palette["lightBack"] + "; }"
          + "QScrollBar:horizontal { background-color: transparent; height: 10px; margin: 0px; border: none; }"
          + "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background: none; }"
          + "QScrollBar:vertical { background-color: " + m_palette["lightBackHover"] + "; border-radius: 3px; margin: 2px; }"
          + "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { width: 0px; background: none; }"
          + "QScrollBar::handle:horizontal, QScrollBar::handle:vertical { background-color: " + m_palette["lightBackHover"] + "; border-radius: 3px; margin: 2px; }"
          + "QScrollBar::handle:horizontal:pressed, QScrollBar::handle:vertical:pressed { background-color: " + m_palette["lightBackPressed"] + "; }"
          + "QAbstractScrollArea::corner { background-color: " + m_palette["lightBack"] + "; }"

          + "QTabBar::tab { background: transparent; border: none; color: " + m_palette["lightBrandFore"] +
          "; font-family: 'Segoe UI'; font-size: 14px; margin: 10px 10px 6px 10px; padding: 0px 0px 4px 16px;}"
          + "QTabBar::tab:hover { border-bottom: 3px solid " + m_palette["lightBackHover"] + " }"
          + "QTabBar::tab:selected { border-bottom: 3px solid " + m_palette["lightBrandBack"] + "; font-weight: 600; }"
          + "QTabBar::close-button { border-radius: 4px; margin: 2px; image: url(:/icon/dismissLight.svg); }"
          + "QTabBar::close-button:hover { background-color: " + m_palette["lightBackHover"] + "; }"
          + "QTabBar::close-button:pressed { background-color: " + m_palette["lightBackPressed"] + "; }"
          + "QTabWidget { background-color: " + m_palette["lightBack"] + "; }"
          + "QTabWidget::pane { border: 1px solid #cccccc; border-radius: 2px; border-top: none; }",

          // Dark
          "QMainWindow { background-color: " + m_palette["darkBack"] + "; }"
          + "QScrollBar:horizontal { background-color: transparent; height: 10px; margin: 0px; border: none; }"
          + "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; background: none; }"
          + "QScrollBar:vertical { background-color: " + m_palette["darkBackHover"] + "; border-radius: 3px; margin: 2px; }"
          + "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { width: 0px; background: none; }"
          + "QScrollBar::handle:horizontal, QScrollBar::handle:vertical { background-color: " + m_palette["darkBackHover"] + "; border-radius: 3px; margin: 2px; }"
          + "QScrollBar::handle:horizontal:pressed, QScrollBar::handle:vertical:pressed { background-color: " + m_palette["darkBackPressed"] + "; }"
          + "QAbstractScrollArea::corner { background-color: " + m_palette["darkBack"] + "; }"

          + "QTabBar::tab { background: transparent; border: none; color: " + m_palette["darkBrandFore"] +
          "; font-family: 'Segoe UI'; font-size: 14px; margin: 10px 10px 6px 10px; padding: 0px 0px 4px 16px;}"
          + "QTabBar::tab:hover { border-bottom: 3px solid " + m_palette["darkBackHover"] + " }"
          + "QTabBar::tab:selected { border-bottom: 3px solid " + m_palette["darkBrandBack"] + "; font-weight: 600; }"
          + "QTabBar::close-button { border-radius: 4px; margin: 2px; image: url(:/icon/dismissDark.svg); }"
          + "QTabBar::close-button:hover { background-color: " + m_palette["darkBackHover"] + "; }"
          + "QTabBar::close-button:pressed { background-color: " + m_palette["darkBackPressed"] + "; }"
          + "QTabWidget { background-color: " + m_palette["darkBack"] + "; }"
          + "QTabWidget::pane { border: 1px solid #cccccc; border-radius: 2px; border-top: none; }"
      }) {
    if (m_theme == Theme::Light) QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    else QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    qApp->setStyleSheet(m_styleSheet[m_theme]);
    KDDockWidgets::Config::self().setViewFactory(new CustomWidgetFactory(m_theme));
    emit themeChanged();
}

int GlobalManager::themeGet() const {
    return m_theme;
}

void GlobalManager::themeSet(const int status) {
    if (m_theme != status) {
        m_theme = status;
        if (status == Theme::Light) QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
        else QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
        qApp->setStyleSheet(m_styleSheet[status]);
        KDDockWidgets::Config::self().setViewFactory(new CustomWidgetFactory(status));
        emit themeChanged();
    }
}
