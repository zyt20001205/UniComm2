#include "scriptModule/codeEditor/replaceWidget.h"

#include <QQmlContext>

// public
ReplaceWidget::ReplaceWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(30);
}

void ReplaceWidget::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("mainTooltip", qvariant_cast<QObject *>(objects["mainWindowTooltip"]));

    rootContext()->setContextProperty("replaceWidget", this);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/scriptModule/codeEditor/replaceWidget.qml"));
    hide();
}

void ReplaceWidget::propertyGet(const QVariantMap &objects) {
    m_textField = qvariant_cast<QObject *>(objects["replaceTextField"]);
    m_replaceTextButton = qvariant_cast<QObject *>(objects["replaceTextButton"]);
    m_replaceAllButton = qvariant_cast<QObject *>(objects["replaceAllButton"]);
}

void ReplaceWidget::textReplace() {
    const auto text = m_textField->property("text").toString();
    emit replaceText(text);
}

void ReplaceWidget::allReplace() {
    const auto text = m_textField->property("text").toString();
    emit replaceAll(text);
}

void ReplaceWidget::replaceEnable(const bool status) const {
    m_replaceTextButton->setProperty("enabled", status);
    m_replaceAllButton->setProperty("enabled", status);
}
