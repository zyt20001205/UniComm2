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

    void dwellShowCodeAction(const QUrl &scriptUrl, const QJsonArray &result) const;

    void dwellHide();

signals:
    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void requestCodeAction(const QUrl &scriptUrl, int lineFrom, int indexFrom, int lineTo, int indexTo);

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
    int m_diagnosticLineFrom{};
    int m_diagnosticLineTo{};
    int m_diagnosticIndexFrom{};
    int m_diagnosticIndexTo{};
    int m_typoLineFrom{};
    int m_typoLineTo{};
    int m_typoIndexFrom{};
    int m_typoIndexTo{};
};

#endif //UNICOMM_DWELLWIDGET_H
