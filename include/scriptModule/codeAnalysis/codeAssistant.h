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

    void codeActionShow(const QUrl &scriptUrl, const QJsonArray &result) const;

    void dwellHide() const;

    void navigationShow(const QVariantHash &navigationSession, const QJsonArray &navigations) const;

    void navigationResponse(const QString &hint) const;

    void positionShow(const QVariantMap &positionSession) const;

    void signatureShow(const QVariantHash &signatureSession, const QJsonObject &signature) const;

signals:
    void addChar(const QUrl &scriptUrl, QChar character);

    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void getText(const QUrl &scriptUrl, int startLine, int startCharacter, int endLine, int endCharacter);

    void insertText(const QUrl &scriptUrl, const QString &text, int line, int index);

    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void insertIndicator(const QUrl &scriptUrl, int type, int startLine, int startCharacter, int endLine, int endCharacter, int time);

    void requestCodeAction(const QUrl &scriptUrl, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    CompletionWidget *m_completionWidget{};
    DwellWidget *m_dwellWidget{};
    NavigationWidget *m_navigationWidget{};
    PositionWidget *m_positionWidget{};
    SignatureWidget *m_signatureWidget{};
};

#endif //UNICOMM_CODEASSISTANT_H
