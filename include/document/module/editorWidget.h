#ifndef UNICOMM_EDITORWIDGET_H
#define UNICOMM_EDITORWIDGET_H

#include <QWidget>

class ScintillaWidget;
class SearchWidget;

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorWidget(const QVariantHash &session, QWidget *parent = nullptr);

    ~EditorWidget() override = default;

    void propertySet(const QVariantHash &objects) const;

    [[nodiscard]] ScintillaWidget* handle() const { return m_scintillaWidget; }

protected:
    void miscInit() const;

    void textInit();

private:
    QVariantHash m_session{};

    ScintillaWidget *m_scintillaWidget{};
    SearchWidget *m_searchWidget{};
};

#endif //UNICOMM_EDITORWIDGET_H
