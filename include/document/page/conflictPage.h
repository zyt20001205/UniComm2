#ifndef UNICOMM_CONFLICTPAGE_H
#define UNICOMM_CONFLICTPAGE_H

#include "basePage.h"
#include "document/module/editorWidget.h"

class ConflictPage final : public BasePage {
    Q_OBJECT

public:
    explicit ConflictPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~ConflictPage() override = default;

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
    QObject *m_saveDialog{};
};

#endif //UNICOMM_CONFLICTPAGE_H
