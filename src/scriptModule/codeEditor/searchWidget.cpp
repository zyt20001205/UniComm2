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
    // hide();
}

void SearchWidget::propertyGet(const QVariantMap &objects) {
    m_searchTextField = qvariant_cast<QObject *>(objects["searchTextField"]);
}

void SearchWidget::textSearch(const QString &text) {
    emit searchText(text);
}

void SearchWidget::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) {
    emit setSearchFlags(matchCase, wholeWord, wordStart, regExp);
}

// protected
void SearchWidget::showEvent(QShowEvent *event) {
    QQuickWidget::showEvent(event);
    QMetaObject::invokeMethod(m_searchTextField, "forceActiveFocus");
}
