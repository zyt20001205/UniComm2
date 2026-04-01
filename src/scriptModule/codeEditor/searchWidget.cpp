#include "scriptModule/codeEditor/searchWidget.h"

#include <QQmlContext>

// public
SearchWidget::SearchWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(30);
}

void SearchWidget::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("mainTooltip", qvariant_cast<QObject *>(objects["mainWindowTooltip"]));

    rootContext()->setContextProperty("searchWidget", this);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/scriptModule/codeEditor/searchWidget.qml"));
    hide();
}

void SearchWidget::propertyGet(const QVariantMap &objects) {
    m_textField = qvariant_cast<QObject *>(objects["searchTextField"]);
    m_label = qvariant_cast<QObject *>(objects["searchLabel"]);
}

void SearchWidget::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) {
    emit setSearchFlags(matchCase, wholeWord, wordStart, regExp);
}

void SearchWidget::searchTextSet(const QString &text) const {
    m_textField->setProperty("text", text);
}

void SearchWidget::searchRequest(const QString &text) {
    emit requestSearch(text);
}

void SearchWidget::searchResponse(const QString &text) const {
    m_label->setProperty("text", text);
}

// protected
void SearchWidget::showEvent(QShowEvent *event) {
    QQuickWidget::showEvent(event);
    setFocus();
    QMetaObject::invokeMethod(m_textField, "forceActiveFocus");
}
