#ifndef UNICOMM_PDFPAGE_H
#define UNICOMM_PDFPAGE_H

#include "basePage.h"

class QQuickWidget;

class PdfPage final: public BasePage {
    Q_OBJECT

public:
    explicit PdfPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~PdfPage() override = default;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void documentSave() override;

private:
    QQuickWidget *m_pdfWidget{};
    QObject *m_pdf{};
};

#endif //UNICOMM_PDFPAGE_H
