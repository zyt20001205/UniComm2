#ifndef UNICOMM_DWELLWIDGET_H
#define UNICOMM_DWELLWIDGET_H

#include <QUrl>
#include <QObject>

class QTextBrowser;

class DwellWidget final : public QObject {
    Q_OBJECT

public:
    explicit DwellWidget(QWidget *parent = nullptr);

    ~DwellWidget() override = default;

    void propertySet(const QVariantMap &objects);

    void diagnosticShow(const QVariantHash &diagnosticSession, const QString &message);

    void hoverShow(const QVariantHash &hoverSession, const QString &message) const;

    void dwellShowCodeAction(const QUrl &scriptUrl, const QJsonArray &result) const;

    void dwellHide() const;

    Q_INVOKABLE void linkClick(const QUrl &commandLine);

    Q_INVOKABLE void textReplace(const QString &text);

signals:
    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void requestCodeAction(const QUrl &scriptUrl, int lineFrom, int indexFrom, int lineTo, int indexTo);

private:
    QObject *m_tooltip{};
    QObject *m_diagnosticTextArea{};
    QObject *m_hoverTextArea{};
    QObject *m_suggestionMenu{};

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
