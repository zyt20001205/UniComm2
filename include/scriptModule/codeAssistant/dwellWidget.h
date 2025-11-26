#ifndef UNICOMM_DWELLWIDGET_H
#define UNICOMM_DWELLWIDGET_H

#include <QUrl>
#include <QWidget>

class QTextBrowser;

class DwellWidget final : public QWidget {
    Q_OBJECT

public:
    explicit DwellWidget(QWidget *parent = nullptr);

    ~DwellWidget() override = default;

    void dwellLeave();

    void dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message);

    void dwellShowHover(const QString &message);

    void dwellHide();

signals:
    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

protected:
    void enterEvent(QEnterEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:
    void dwellShowSuggestions(const QStringList &suggestions);

    QTextBrowser *m_diagnosticTextBrowser{};
    QTextBrowser *m_hoverTextBrowser{};
    QMenu *m_suggestionMenu{};

    QUrl m_scriptUrl{};
    int m_lineFrom{};
    int m_lineTo{};
    int m_indexFrom{};
    int m_indexTo{};
};

#endif //UNICOMM_DWELLWIDGET_H
