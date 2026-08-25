#ifndef UNICOMM_MARKUPPAGE_H
#define UNICOMM_MARKUPPAGE_H

#include "documentPage.h"
#include "document/module/editorWidget.h"

class WebviewWidget;

class MarkupPage final : public DocumentPage {
    Q_OBJECT

public:
    explicit MarkupPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~MarkupPage() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] ScintillaWidget *handler() const { return m_editorWidget->handler(); }

    void documentSave() override;

    bool documentClose(bool force = false) override;

signals:
    void changeSelection(const QHash<QString, int> &selection);

private:
    void previewUpdate();

    void savepointChange(bool status);

    EditorWidget *m_editorWidget{};
    WebviewWidget *m_webviewWidget{};
    QObject *m_saveDialog{};
};

#endif //UNICOMM_MARKUPPAGE_H
