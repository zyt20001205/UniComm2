#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QSortFilterProxyModel>
#include <QWidget>

class QLabel;
class QListView;
class QPushButton;
class QStandardItemModel;

class CompletionWidget final : public QWidget {
    Q_OBJECT

public:
    explicit CompletionWidget(QWidget *parent = nullptr);

    ~CompletionWidget() override = default;

    void completionShow(const QVariantMap &completionSession, const QJsonArray &items);

    void completionHide();

    void completionPrev() const;

    void completionNext() const;

    void textReplace();

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void addChar(const QUrl &scriptUrl, QChar character);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    void filterClear() const;

    void filterInit(int mode);

    void filterSet(bool status);

    void labelShow() const;

    QVariantMap m_completionSession{};
    QListView *m_completionListView{};
    QStandardItemModel *m_completionModel{};
    QSortFilterProxyModel *m_filterProxyModel{};
    QSet<int> m_completionKinds{};
    QWidget *m_filterWidget{};
    QHash<int, QPushButton *> m_filterButtonHash{};
    QPushButton *m_textButton{};
    QPushButton *m_functionButton{};
    QPushButton *m_fieldButton{};
    QPushButton *m_variableButton{};
    QPushButton *m_enumButton{};
    QPushButton *m_keywordButton{};
    QPushButton *m_enummemberButton{};
    QPushButton *m_resetButton{};
    QLabel *m_completionLabel{};

    enum {
        COMPLETION_MODE_FULL,
        COMPLETION_MODE_SIMPLE
    };
};

#endif //UNICOMM_COMPLETIONPOPUP_H
