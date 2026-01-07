#include "scriptModule/codeDebug/watchModule.h"

#include <QQmlContext>
#include <QQuickWidget>

// WatchModule public
WatchModule::WatchModule()
    : DockWidget("welcome"),
      m_watchWidget(new QQuickWidget()) {
    setWidget(m_watchWidget);
}

WatchModule::~WatchModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] watch module destructed").arg(timestamp);
}

void WatchModule::propertySet(const QVariantMap &objects) {
    m_watchWidget->rootContext()->setContextProperty("watchModule", this);
    m_watchWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_watchWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/watchModule.qml"));
}

void WatchModule::watchInsert() {
}
