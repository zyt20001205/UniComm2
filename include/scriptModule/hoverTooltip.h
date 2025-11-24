#ifndef UNICOMM_HOVERTOOLTIP_H
#define UNICOMM_HOVERTOOLTIP_H

#include <QUrl>
#include <QWidget>

class QTextBrowser;

class HoverTooltip final : public QWidget {
    Q_OBJECT

public:
    explicit HoverTooltip(QWidget *parent = nullptr);

    ~HoverTooltip() override = default;

    void tooltipLeave();

    void tooltipShowDiagnostic(const QUrl &scriptUrl, const QString &message);

    void tooltipShowHover(const QString &message);

    void tooltipHide();

signals:
    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

protected:
    void enterEvent(QEnterEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void leaveEvent(QEvent *event) override;

    void toolTipShowSuggestions(const QStringList &suggestions);

private:
    QTextBrowser *m_diagnosticTextBrowser{};
    QTextBrowser *m_hoverTextBrowser{};
    QMenu *m_suggestionMenu{};

    QUrl m_scriptUrl{};
    int m_lineFrom{};
    int m_lineTo{};
    int m_indexFrom{};
    int m_indexTo{};
};

#endif //UNICOMM_HOVERTOOLTIP_H
