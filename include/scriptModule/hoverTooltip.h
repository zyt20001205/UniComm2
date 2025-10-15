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

    void showTooltip(const QString &message);

    void hideTooltip();

signals:
    void switchDwell(bool status);

private:
    QTextBrowser *m_textBrowser = nullptr;
    QPointer<QWidget> m_previousFocus = nullptr;
};


#endif //UNICOMM_HOVERTOOLTIP_H