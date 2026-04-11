#include "data/dataplotModule.h"

#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"

// public
DataplotModule::DataplotModule()
    : DockWidget("Dataplot"),
      m_dataplotWidget(new QQuickWidget()) {
    setWidget(m_dataplotWidget);
}

DataplotModule::~DataplotModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] dataplot module destructed").arg(timestamp);
}

void DataplotModule::propertySet(const QVariantMap &objects) {
    m_dataplotWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["dataplotModuleRootMenu"]));

    m_dataplotWidget->rootContext()->setContextProperty("dataplotModule", this);
    m_dataplotWidget->rootContext()->setContextProperty("databaseStandardItemModel", g_databaseStandardItemModel);
    m_dataplotWidget->rootContext()->setContextProperty("datatableHeaderItemModel", g_datatableHeaderItemModel);
    m_dataplotWidget->rootContext()->setContextProperty("datatableStandardItemModel", g_datatableStandardItemModel);
    m_dataplotWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_dataplotWidget->setSource(QUrl("qrc:/qml/data/dataplotModule.qml"));
}

// private
