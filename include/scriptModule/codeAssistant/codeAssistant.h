#ifndef UNICOMM_CODEASSISTANT_H
#define UNICOMM_CODEASSISTANT_H
#include <QObject>

class CompletionWidget;
class DwellWidget;
class GotoWidget;
class PositionWidget;
class SignatureWidget;

class CodeAssistant final: public QObject {
    Q_OBJECT

public:
    explicit CodeAssistant(QWidget *parent = nullptr);

    ~CodeAssistant() override = default;

    void completionShow(const QVariantMap &completionSession, const QJsonArray &items) const;

    void completionHide() const;

    void dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message) const;

    void dwellShowHover(const QString &message) const;

    void dwellHide() const;

    void dwellLeave() const;

    void gotoShowDefinition(const QVariantMap &gotoSession, const QJsonArray &definitions) const;

    void gotoShowImplementation(const QVariantMap &gotoSession, const QJsonArray &implementations) const;

    void gotoShowReferences(const QVariantMap &gotoSession, const QJsonArray &references) const;

    void gotoShowTypeDefinition(const QVariantMap &gotoSession, const QJsonArray &typeDefinitions) const;

    void positionShow(const QVariantMap &positionSession) const;

    void signatureShow(const QVariantMap &signatureSession, const QJsonObject &signature) const;

signals:
    void addChar(const QUrl &scriptUrl, QChar character);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void insertText(const QUrl &scriptUrl, const QString &text, int line, int index);

    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    CompletionWidget *m_completionWidget{};
    DwellWidget *m_dwellWidget{};
    GotoWidget *m_gotoWidget{};
    PositionWidget *m_positionWidget{};
    SignatureWidget *m_signatureWidget{};
};

#endif //UNICOMM_CODEASSISTANT_H
