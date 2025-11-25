#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QListView>
#include <QStandardItemModel>
#include <QWidget>

class QLabel;

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
    void moveUp();

    void moveDown();

    void codeComplete();

    void labelShow();

    QListView *m_completionListView;
    QStandardItemModel *m_completionModel{};
    QLabel *m_completionLabel{};
    bool m_fullComplete = false;
};

#endif //UNICOMM_COMPLETIONPOPUP_H