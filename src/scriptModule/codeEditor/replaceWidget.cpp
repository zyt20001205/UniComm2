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
    m_replaceButton = qvariant_cast<QObject *>(objects["replaceButton"]);
    m_replaceAllButton = qvariant_cast<QObject *>(objects["replaceAllButton"]);
}

void ReplaceWidget::textReplace() {
    const auto text = m_textField->property("text").toString();
    emit replaceText(text);
}

void ReplaceWidget::textReplaceAll() {
    const auto text = m_textField->property("text").toString();
    qDebug() << "replace all" << text;
}

void ReplaceWidget::replaceEnable(const bool status) const {
    m_replaceButton->setProperty("enabled", status);
    m_replaceAllButton->setProperty("enabled", status);
}
