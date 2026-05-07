#ifndef UNICOMM_EDITORWIDGET_H
#define UNICOMM_EDITORWIDGET_H

#include <QWidget>

class ScintillaWidget;
class SearchWidget;

class EditorWidget final : public QWidget {
    Q_OBJECT

public:
    explicit EditorWidget(const QVariantHash &session, QWidget *parent = nullptr);

    ~EditorWidget() override = default;

    [[nodiscard]] ScintillaWidget* handle() const { return m_scintillaWidget; }

private:
    ScintillaWidget *m_scintillaWidget{};
    SearchWidget *m_searchWidget{};
};

#endif //UNICOMM_EDITORWIDGET_H
