#ifndef UNICOMM_DWELLWIDGET_H
#define UNICOMM_DWELLWIDGET_H

#include <QUrl>
#include <QObject>

class QTextBrowser;

class DwellWidget final : public QObject {
    Q_OBJECT

public:
    explicit DwellWidget(QObject *parent = nullptr);

    ~DwellWidget() override = default;

    void propertySet(const QVariantHash &objects);

    void diagnosticShow(const QVariantHash &diagnosticSession, const QString &message);

    void hoverShow(const QVariantHash &hoverSession, const QString &message) const;

    void codeActionShow(const QUrl &documentUrl, const QJsonArray &result) const;

    Q_INVOKABLE void dwellHide() const;

    Q_INVOKABLE void linkClick(const QUrl &customUrl);

    Q_INVOKABLE void suggestionAccept(const QString &text);

    Q_INVOKABLE void codeActionAccept(const QJsonObject &codeAction);

signals:
    void textSet(const QUrl &documentUrl, const QString &text, int startLine, int startCharacter, int endLine, int endCharacter);

    void requestCodeAction(const QUrl &documentUrl, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    QObject *m_tooltip{};
    QObject *m_diagnosticTextArea{};
    QObject *m_hoverTextArea{};
    QObject *m_codeActionMenu{};
    QObject *m_suggestionMenu{};

    QUrl m_documentUrl{};
    int m_diagnosticStartLine{};
    int m_diagnosticEndLine{};
    int m_diagnosticStartCharacter{};
    int m_diagnosticEndCharacter{};
    int m_typoStartLine{};
    int m_typoEndLine{};
    int m_typoStartCharacter{};
    int m_typoEndCharacter{};
};

#endif //UNICOMM_DWELLWIDGET_H
