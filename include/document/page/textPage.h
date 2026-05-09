#ifndef UNICOMM_TEXTPAGE_H
#define UNICOMM_TEXTPAGE_H

#include "basePage.h"
#include "document/module/editorWidget.h"

class EditorWidget;

class TextPage final : public BasePage {
    Q_OBJECT

public:
    explicit TextPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~TextPage() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] ScintillaWidget* handler() const { return m_editorWidget->handler();}

    void documentSave() const;

    void selectionChange() const;

signals:
    void changeSelection(const QHash<QString, int> &selection);

protected:
    bool documentClose() override;

private:
    void savepointChange(bool status);

    EditorWidget *m_editorWidget{};
    QObject *m_saveDialog{};
};

#endif //UNICOMM_TEXTPAGE_H
