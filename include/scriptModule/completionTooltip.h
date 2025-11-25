#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QSortFilterProxyModel>
#include <QWidget>

class QLabel;
class QListView;
class QPushButton;
class QStandardItemModel;

class CompletionTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit CompletionTooltip(QWidget *parent = nullptr);

    ~CompletionTooltip() override = default;

    void tooltipShow(const QJsonArray &items);

    void tooltipHide();

    void tooltipFull(bool status);

signals:
    void completeCode(QString &text, int kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    void hideEvent(QHideEvent *event) override;

private:
    void moveUp() const;

    void moveDown() const;

    void codeComplete();

    void filterClear() const;

    void filterInit();

    void filterSet(bool status);

    void labelShow(const QModelIndex &currentIndex, const QModelIndex &previousIndex) const;

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
    bool m_fullComplete = false;
};

#endif //UNICOMM_COMPLETIONPOPUP_H
