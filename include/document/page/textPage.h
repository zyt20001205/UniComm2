#ifndef UNICOMM_TEXTPAGE_H
#define UNICOMM_TEXTPAGE_H

#include "basePage.h"

class EditorWidget;

class TextPage final : public BasePage {
    Q_OBJECT

public:
    explicit TextPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~TextPage() override = default;

    void propertySet(const QVariantHash &objects) const;

    void documentSave() const;

signals:
    void changeSelection(const QHash<QString, int> &selection);

private:
    void savepointChange(bool status);

    EditorWidget *m_editorWidget{};
};

#endif //UNICOMM_TEXTPAGE_H
