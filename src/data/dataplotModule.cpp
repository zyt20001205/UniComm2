#include "data/dataplotModule.h"

#include <QQmlContext>
#include <QQuickWidget>

#include "globals.h"
#include "core/globalManager.h"

// public
DataplotModule::DataplotModule()
    : DockWidget("Dataplot"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
}

DataplotModule::~DataplotModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void DataplotModule::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("dataplotModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["dataplotModuleRootMenu"]));
    m_widget->rootContext()->setContextProperty("databaseStandardItemModel", g_databaseStandardItemModel);
    m_widget->rootContext()->setContextProperty("datatableHeaderItemModel", g_datatableHeaderItemModel);
    m_widget->rootContext()->setContextProperty("datatableStandardItemModel", g_datatableStandardItemModel);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/data/dataplotModule.qml"));
}

// private
