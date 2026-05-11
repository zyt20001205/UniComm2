#ifndef UNICOMM_CODEWIDGET_H
#define UNICOMM_CODEWIDGET_H

#include "editorWidget.h"

class EditorWidget;
class SymbolWidget;

class CodeWidget : public QWidget {
    Q_OBJECT

public:
    explicit CodeWidget(const QJsonObject &documentConfig, const QUrl &documentUrl, QWidget *parent = nullptr);

    ~CodeWidget() override = default;

    void propertySet(const QVariantHash &objects);

    [[nodiscard]] ScintillaWidget *handler() const { return m_editorWidget->handler(); }

private:
    EditorWidget *m_editorWidget{};
    SymbolWidget *m_symbolWidget{};
};

#endif //UNICOMM_CODEWIDGET_H
