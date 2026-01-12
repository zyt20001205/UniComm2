#ifndef UNICOMM_POSITIONWIDGET_H
#define UNICOMM_POSITIONWIDGET_H

#include <QWidget>

class QLabel;

class PositionWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PositionWidget(QWidget *parent = nullptr);

    ~PositionWidget() override = default;

    void positionShow(const QVariantMap &positionSession);

    void positionHide();

    void textReplace();

signals:
    void insertText(const QUrl &scriptUrl, const QString &text, int line, int index);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    QVariantMap m_positionSession{};
    QTimer *m_timer{};
    QLabel *m_label{};
};

#endif //UNICOMM_POSITIONWIDGET_H
