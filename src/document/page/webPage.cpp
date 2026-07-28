#include "document/page/webPage.h"

#include "document/module/webviewWidget.h"

// public
WebPage::WebPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : DocumentPage(documentUrl),
      m_webviewWidget(new WebviewWidget()) {
    setWidget(m_webviewWidget);
    m_webviewWidget->navigate(documentUrl);
}

void WebPage::propertySet(const QVariantHash &objects) {
}

void WebPage::documentSave() {
}
