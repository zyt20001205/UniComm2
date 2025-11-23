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

    void tooltipShowDiagnostic(const QString &message);

    void tooltipShowTypo(const QString &word, const QStringList &suggestions);

    void tooltipShowHover(const QString &message);

    void tooltipHide();

    void tooltipResize();

protected:
    void enterEvent(QEnterEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:
    QTextBrowser *m_diagnosticTextBrowser{};
    QTextBrowser *m_hoverTextBrowser{};
};

#endif //UNICOMM_HOVERTOOLTIP_H