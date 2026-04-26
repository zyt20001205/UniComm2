#include "terminal/cmdModule.h"

#include <QQmlContext>
#include <QQuickWidget>
#include <QTextDocument>

#include "globals.h"

// public
CmdModule::CmdModule()
    : DockWidget("CMD"),
      m_config(g_workspaceConfig["logConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()) {
    setWidget(m_widget);
}

CmdModule::~CmdModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void CmdModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("cmdModule", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/cmdModule.qml"));
}

void CmdModule::propertyGet(const QVariantMap &objects) {
}
