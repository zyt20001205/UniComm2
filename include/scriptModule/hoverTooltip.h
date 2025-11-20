#ifndef UNICOMM_HOVERTOOLTIP_H
#define UNICOMM_HOVERTOOLTIP_H

#include <QPointer>
#include <QWidget>

class QTextBrowser;

class HoverTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit HoverTooltip(QWidget *parent = nullptr);

    ~HoverTooltip() override = default;

    void tooltipLeave();

    void tooltipShow(const QString &message);

    void tooltipHide();

protected:
    void enterEvent(QEnterEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:
    QTextBrowser *m_textBrowser{};
};

#endif //UNICOMM_HOVERTOOLTIP_H