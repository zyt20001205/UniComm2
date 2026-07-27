#ifndef UNICOMM_MARKDOWNPAGE_H
#define UNICOMM_MARKDOWNPAGE_H

#include "documentPage.h"
#include "document/module/editorWidget.h"

class WebviewWidget;

class MarkdownPage final : public DocumentPage {
    Q_OBJECT

public:
    explicit MarkdownPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~MarkdownPage() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] ScintillaWidget *handler() const { return m_editorWidget->handler(); }

    void documentSave() override;

    bool documentClose(bool force = false) override;

    void documentGoto() const;

signals:
    void changeSelection(const QHash<QString, int> &selection);

private:
    void savepointChange(bool status);

    EditorWidget *m_editorWidget{};
    WebviewWidget *m_webviewWidget{};
    QObject *m_saveDialog{};
};

#endif //UNICOMM_MARKDOWNPAGE_H
