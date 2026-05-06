#include "document/page/welcomePage.h"

#include <QQmlContext>
#include <QQuickWidget>

// public
WelcomePage::WelcomePage()
    : DockWidget("welcome") ,
    m_welcomeWidget(new QQuickWidget()){
    setWidget(m_welcomeWidget);
}

void WelcomePage::propertySet(const QVariantMap &objects) {
    m_welcomeWidget->rootContext()->setContextProperty("welcomePage", this);
    m_welcomeWidget->rootContext()->setContextProperty("global", objects["global"]);

    m_welcomeWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_welcomeWidget->setSource(QUrl("qrc:/qml/document/page/welcomePage.qml"));
}

void WelcomePage::workspaceOpen() {
    emit openWorkspace();
}
