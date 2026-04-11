#include "document/module/searchWidget.h"

#include <QQmlContext>

// public
SearchWidget::SearchWidget(QWidget *parent)
    : QQuickWidget(parent) {
    setFixedHeight(30);
    hide();
}

void SearchWidget::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));

    rootContext()->setContextProperty("searchWidget", this);
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/document/module/searchWidget.qml"));
}

void SearchWidget::propertyGet(const QVariantMap &objects) {
    m_textField = qvariant_cast<QObject *>(objects["searchTextField"]);
    m_prevButton = qvariant_cast<QObject *>(objects["prevButton"]);
    m_nextButton = qvariant_cast<QObject *>(objects["nextButton"]);
    m_label = qvariant_cast<QObject *>(objects["searchLabel"]);
}

void SearchWidget::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) {
    emit setSearchFlags(matchCase, wholeWord, wordStart, regExp);
}

void SearchWidget::searchRequest() {
    const auto text = m_textField->property("text").toString();
    emit requestSearch(text);
}

void SearchWidget::searchRequest(const QString &text) {
    m_textField->setProperty("text", text);
    searchRequest();
}

void SearchWidget::searchResponse(const QString &text) const {
    m_label->setProperty("text", text);
}

void SearchWidget::searchPrev() {
    emit prevSearch();
}

void SearchWidget::searchNext() {
    emit nextSearch();
}

void SearchWidget::searchEnable(const bool status) const {
    m_prevButton->setProperty("enabled", status);
    m_nextButton->setProperty("enabled", status);
}

// protected
void SearchWidget::showEvent(QShowEvent *event) {
    QQuickWidget::showEvent(event);
    setFocus();
    QMetaObject::invokeMethod(m_textField, "forceActiveFocus");
}

void SearchWidget::hideEvent(QHideEvent *event) {
    if (m_textField) searchRequest(QString());
    QQuickWidget::hideEvent(event);
}
