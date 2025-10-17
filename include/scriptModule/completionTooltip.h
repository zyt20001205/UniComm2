#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QWidget>

class QTableWidget;

class CompletionTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit CompletionTooltip(QWidget *parent = nullptr);

    ~CompletionTooltip() override = default;

    void showTooltip(const QJsonArray &items);

    void hideTooltip();

    void fullCompleteSet(bool status);

signals:
    void replaceText(QString &text, const QString &kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void moveUp();

    void moveDown();

    QTableWidget *m_tableWidget = nullptr;
    bool m_fullComplete = false;
    int m_currentRow{};
    QString m_insertText{};
    QString m_kind{};
    QList<QString> m_kindList{};
};

#endif //UNICOMM_COMPLETIONPOPUP_H