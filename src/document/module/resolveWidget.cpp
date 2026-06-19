#include "document/module/resolveWidget.h"

#include <QColor>
#include <QQmlContext>

#include "globals.h"
#include "core/globalManager.h"

// public
ResolveWidget::ResolveWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedSize(160, 36);
    setClearColor(Qt::transparent);
    setAttribute(Qt::WA_AlwaysStackOnTop);
}

void ResolveWidget::propertySet(const QVariantHash &objects) {
    rootContext()->setContextProperty("resolveWidget", this);
    rootContext()->setContextProperty("global", g_globalManager);
    rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/document/module/resolveWidget.qml"));
}

void ResolveWidget::propertyGet(const QVariantMap &objects) {
    m_resolvePrevButton = qvariant_cast<QObject *>(objects["resolvePrevButton"]);
    m_resolveNextButton = qvariant_cast<QObject *>(objects["resolveNextButton"]);
    m_resolveStatLabel = qvariant_cast<QObject *>(objects["resolveStatLabel"]);
}

void ResolveWidget::resolvePrev() {
    emit prevResolve();
}

void ResolveWidget::resolveNext() {
    emit nextResolve();
}

