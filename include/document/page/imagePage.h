#ifndef UNICOMM_IMAGEPAGE_H
#define UNICOMM_IMAGEPAGE_H

#include "documentPage.h"

class QQuickWidget;

class ImagePage final: public DocumentPage {
    Q_OBJECT

public:
    explicit ImagePage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~ImagePage() override = default;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

private:
    QQuickWidget *m_imageWidget{};
    QObject *m_image{};
};

#endif //UNICOMM_IMAGEPAGE_H
