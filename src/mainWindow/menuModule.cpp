#include "mainWindow/menuModule.h"

#include <QQmlContext>
#include <QQuickItem>

#include "globals.h"
#include "core/globalManager.h"

// public
MenuModule::MenuModule(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(24);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

MenuModule::~MenuModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] menu module destructed").arg(timestamp);
}

void MenuModule::propertySet(const QVariantHash &objects) {
    rootContext()->setContextProperty("menuModule", this);
    rootContext()->setContextProperty("global", g_globalManager);
    rootContext()->setContextProperty("fileMenu", qvariant_cast<QObject *>(objects["menuModuleFileMenu"]));
    rootContext()->setContextProperty("editMenu", qvariant_cast<QObject *>(objects["menuModuleEditMenu"]));
    rootContext()->setContextProperty("viewMenu", qvariant_cast<QObject *>(objects["menuModuleViewMenu"]));
    rootContext()->setContextProperty("navMenu", qvariant_cast<QObject *>(objects["menuModuleNavMenu"]));
    rootContext()->setContextProperty("codeMenu", qvariant_cast<QObject *>(objects["menuModuleCodeMenu"]));
    rootContext()->setContextProperty("execMenu", qvariant_cast<QObject *>(objects["menuModuleExecMenu"]));
    rootContext()->setContextProperty("gitMenu", qvariant_cast<QObject *>(objects["menuModuleGitMenu"]));

    setSource(QUrl("qrc:/qml/mainWindow/menuModule.qml"));
    setVisible(true);
}

void MenuModule::themeSet(const int theme) {
    emit setTheme(theme);
}
