#ifndef UNICOMM_CONFLICTWIDGET_H
#define UNICOMM_CONFLICTWIDGET_H

#include <QJsonArray>

#include "editorWidget.h"
#include "ScintillaTypes.h"

class EditorWidget;

class ConflictWidget final : public EditorWidget {
    Q_OBJECT

public:
    explicit ConflictWidget(const QJsonObject &documentConfig, const QUrl &documentUrl, QWidget *parent = nullptr);

    ~ConflictWidget() override = default;

    void propertySet(const QVariantHash &objects) override;

signals:

protected:
    void marginInit() const override;

    void markerInit() const override;

private:
    void marginClick(Scintilla::Position position, int mouseButton, Scintilla::KeyMod modifiers, int margin);

    void contentChange();

    QTimer *m_contentTimer{};
};

#endif //UNICOMM_CONFLICTWIDGET_H
