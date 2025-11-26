#ifndef UNICOMM_CODEASSISTANT_H
#define UNICOMM_CODEASSISTANT_H
#include <QObject>

class CompletionWidget;
class DwellWidget;

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

signals:
    void setCursorPosition(const QUrl &scriptUrl, int startLine, int startCharacter);

    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

    void addChar(const QUrl &scriptUrl, QChar character);

    void insertPort();

    void insertDatabase();

    void insertDatatable();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    CompletionWidget *m_completionWidget{};
    DwellWidget *m_dwellWidget;
};

#endif //UNICOMM_CODEASSISTANT_H
