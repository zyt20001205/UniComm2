#ifndef UNICOMM_CODEASSISTANT_H
#define UNICOMM_CODEASSISTANT_H

#include <QObject>

class CompletionWidget;
class DwellWidget;
class NavigationWidget;
class PositionWidget;
class SignatureWidget;

class CodeAssistant final: public QObject {
    Q_OBJECT

public:
    explicit CodeAssistant(QWidget *parent = nullptr);

    ~CodeAssistant() override = default;

    void propertySet(const QVariantMap &objects) const;

    void fontSet(const QString &family, int pointSize) const;

    void completionShow(const QVariantHash &completionSession, const QJsonArray &items) const;

    void completionHide() const;

    void diagnosticShow(const QVariantHash &diagnosticSession, const QString &message) const;

    void hoverShow(const QVariantHash &hoverSession, const QString &message) const;

    void codeActionShow(const QUrl &documentUrl, const QJsonArray &result) const;

    void dwellHide() const;

    void navigationShow(const QVariantHash &navigationSession, const QJsonArray &navigations) const;

    void positionShow(const QVariantMap &positionSession) const;

    void signatureShow(const QVariantHash &signatureSession, const QJsonArray &signatures) const;

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void appendLog(const QString &message, int type);

    void addChar(const QUrl &documentUrl, QChar character);

    void setIndex(const QUrl &documentUrl, int line, int character);

    void setText(const QUrl &documentUrl, const QString &text, int startLine, int startCharacter, int endLine, int endCharacter);

    void setTextSelected(const QUrl &documentUrl, const QString &text);

    void getText(const QUrl &documentUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void insertIndicator(const QUrl &documentUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

    void requestCodeAction(const QUrl &documentUrl, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    CompletionWidget *m_completionWidget{};
    DwellWidget *m_dwellWidget{};
    NavigationWidget *m_navigationWidget{};
    PositionWidget *m_positionWidget{};
    SignatureWidget *m_signatureWidget{};
};

#endif //UNICOMM_CODEASSISTANT_H
