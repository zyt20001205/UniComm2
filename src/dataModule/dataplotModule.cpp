#include "dataModule/dataplotModule.h"

#include <QQmlContext>
#include <QQuickWidget>

// DataplotModule public
DataplotModule::DataplotModule()
    : DockWidget("dataplot"),
      m_dataplotWidget(new QQuickWidget()) {
    setWidget(m_dataplotWidget);
}

DataplotModule::~DataplotModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] dataplot module destructed").arg(timestamp);
}

void DataplotModule::propertySet(const QVariantMap &objects) {
    m_dataplotWidget->rootContext()->setContextProperty("dataplotModule", this);
    m_dataplotWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_dataplotWidget->setSource(QUrl("qrc:/qml/dataModule/dataplotModule.qml"));
}

// DataplotModule private
