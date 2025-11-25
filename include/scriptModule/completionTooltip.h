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
    void completeCode(QString &text, int kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void moveUp() const;

    void moveDown() const;

    void codeComplete();

    QTableWidget *m_tableWidget{};
    bool m_fullComplete = false;
};

#endif //UNICOMM_COMPLETIONPOPUP_H