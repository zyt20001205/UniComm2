#ifndef UNICOMM_WEBPAGE_H
#define UNICOMM_WEBPAGE_H

#include "documentPage.h"

class WebviewWidget;

class WebPage final : public DocumentPage {
    Q_OBJECT

public:
    explicit WebPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~WebPage() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] WebviewWidget *handler() const { return m_webviewWidget; }

    void documentSave() override;

private:
    WebviewWidget *m_webviewWidget{};
};

#endif //UNICOMM_WEBPAGE_H
