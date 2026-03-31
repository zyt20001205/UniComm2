#include "scriptModule/codeEditor/replaceWidget.h"

#include <QQmlContext>

// public
ReplaceWidget::ReplaceWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(30);
}

void ReplaceWidget::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("replaceWidget", this);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/scriptModule/codeEditor/replaceWidget.qml"));
    hide();
}

void ReplaceWidget::propertyGet(const QVariantMap &objects) {
}

void ReplaceWidget::textReplace(const QString &text) {
    qDebug() << "replace" << text;
}

void ReplaceWidget::textReplaceAll(const QString &text) {
    qDebug() << "replace all" << text;
}
