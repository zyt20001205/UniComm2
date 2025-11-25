#ifndef UNICOMM_POSITIONTOOLTIP_H
#define UNICOMM_POSITIONTOOLTIP_H

#include <QWidget>

class QLabel;

class PositionTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit PositionTooltip(QWidget *parent = nullptr);

    ~PositionTooltip() override = default;

    void tooltipShow();

    void tooltipHide();

signals:
    void replaceText(QString &text, int kind);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTimer *m_timer{};
    QLabel *m_label{};
};

#endif //UNICOMM_POSITIONTOOLTIP_H
