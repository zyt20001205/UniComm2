#include "document/page/imagePage.h"

#include <QQmlContext>
#include <QQuickWidget>

// public
ImagePage::ImagePage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_imageWidget(new QQuickWidget()) {
    setWidget(m_imageWidget);
}

void ImagePage::propertySet(const QVariantMap &objects) {
    m_imageWidget->rootContext()->setContextProperty("imagePage", this);
    m_imageWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_imageWidget->setSource(QUrl("qrc:/qml/document/module/imageWidget.qml"));
}

void ImagePage::propertyGet(const QVariantMap &objects) {
    m_image = qvariant_cast<QObject *>(objects["image"]);
    m_image->setProperty("source", m_documentUrl);
}
