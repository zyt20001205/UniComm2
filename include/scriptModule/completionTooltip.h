#ifndef UNICOMM_COMPLETIONPOPUP_H
#define UNICOMM_COMPLETIONPOPUP_H

#include <QWidget>

class QTableWidget;

class CompletionTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit CompletionTooltip(QWidget *parent = nullptr);

    ~CompletionTooltip() override = default;

    void tooltipShow(const QJsonArray &items);

    void tooltipHide();

    void tooltipFull(bool status);

signals:
    void replaceText(QString &text, const QString &kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void moveUp() const;

    void moveDown() const;

    void textReplace();

    QTableWidget *m_tableWidget = nullptr;
    bool m_fullComplete = false;
    QList<QString> m_kindList{};
};

#endif //UNICOMM_COMPLETIONPOPUP_H