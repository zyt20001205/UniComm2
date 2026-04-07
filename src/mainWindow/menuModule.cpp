#include "mainWindow/menuModule.h"

#include <QQmlContext>

// public
MenuModule::MenuModule(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(24);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

MenuModule::~MenuModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] menu module destructed").arg(timestamp);
}

void MenuModule::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("fileMenu", qvariant_cast<QObject *>(objects["menuModuleFileMenu"]));
    rootContext()->setContextProperty("viewMenu", qvariant_cast<QObject *>(objects["menuModuleViewMenu"]));
    rootContext()->setContextProperty("codeMenu", qvariant_cast<QObject *>(objects["menuModuleCodeMenu"]));

    rootContext()->setContextProperty("menuModule", this);
    setSource(QUrl("qrc:/qml/mainWindow/menuModule.qml"));
    setVisible(true);
}