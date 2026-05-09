#include "document/module/searchWidget.h"

#include <QQmlContext>

// public
SearchWidget::SearchWidget(QWidget *parent)
    : QQuickWidget(parent) {
    hide();
    installEventFilter(this);
}

void SearchWidget::propertySet(const QVariantHash &objects) {
    rootContext()->setContextProperty("searchWidget", this);
    rootContext()->setContextProperty("global", objects["global"]);
    rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));

    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/document/module/searchWidget.qml"));
}

void SearchWidget::propertyGet(const QVariantMap &objects) {
    m_searchBar = qvariant_cast<QObject *>(objects["searchBar"]);
    m_searchTextField = qvariant_cast<QObject *>(objects["searchTextField"]);
    m_searchPrevButton = qvariant_cast<QObject *>(objects["searchPrevButton"]);
    m_searchNextButton = qvariant_cast<QObject *>(objects["searchNextButton"]);
    m_searchStatLabel = qvariant_cast<QObject *>(objects["searchStatLabel"]);
    m_replaceBar = qvariant_cast<QObject *>(objects["replaceBar"]);
    m_replaceTextField = qvariant_cast<QObject *>(objects["replaceTextField"]);
    m_replaceTextButton = qvariant_cast<QObject *>(objects["replaceTextButton"]);
    m_replaceAllButton = qvariant_cast<QObject *>(objects["replaceAllButton"]);
}

void SearchWidget::searchShow() {
    m_searchBar->setProperty("visible", true);
    m_replaceBar->setProperty("visible", false);
    setFixedHeight(36);
}

void SearchWidget::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) {
    emit setSearchFlags(matchCase, wholeWord, wordStart, regExp);
}

void SearchWidget::searchRequest() {
    const auto text = m_searchTextField->property("text").toString();
    emit requestSearch(text);
}

void SearchWidget::searchRequest(const QString &text) {
    m_searchTextField->setProperty("text", text);
    QMetaObject::invokeMethod(m_searchTextField, "selectAll");
    searchRequest();
}

void SearchWidget::searchResponse(const QString &text) const {
    m_searchStatLabel->setProperty("text", text);
}

void SearchWidget::searchPrev() {
    emit prevSearch();
}

void SearchWidget::searchNext() {
    emit nextSearch();
}

void SearchWidget::searchEnable(const bool status) const {
    m_searchPrevButton->setProperty("enabled", status);
    m_searchNextButton->setProperty("enabled", status);
}

void SearchWidget::replaceShow() {
    m_searchBar->setProperty("visible", true);
    m_replaceBar->setProperty("visible", true);
    setFixedHeight(64);
}

void SearchWidget::textReplace() {
    const auto text = m_replaceTextField->property("text").toString();
    emit replaceText(text);
}

void SearchWidget::allReplace() {
    const auto text = m_replaceTextField->property("text").toString();
    emit replaceAll(text);
}

void SearchWidget::replaceEnable(const bool status) const {
    m_replaceTextButton->setProperty("enabled", status);
    m_replaceAllButton->setProperty("enabled", status);
}

bool SearchWidget::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_Escape:
                if (isVisible()) hide();
                return false;
        default: return false;
        }
    }
    return QQuickWidget::eventFilter(watched, event);
}

// protected
void SearchWidget::showEvent(QShowEvent *event) {
    QQuickWidget::showEvent(event);
    setFocus();
    QMetaObject::invokeMethod(m_searchTextField, "forceActiveFocus");
}

void SearchWidget::hideEvent(QHideEvent *event) {
    if (m_searchTextField) searchRequest(QString());
    QQuickWidget::hideEvent(event);
}
