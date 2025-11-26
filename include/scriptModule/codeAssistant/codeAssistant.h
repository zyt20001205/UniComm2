#ifndef UNICOMM_CODEASSISTANT_H
#define UNICOMM_CODEASSISTANT_H
#include <QObject>

class DwellWidget;

class CodeAssistant final: public QObject {
    Q_OBJECT

public:
    explicit CodeAssistant(QWidget *parent = nullptr);

    ~CodeAssistant() override = default;

    void dwellShowDiagnostic(const QUrl &scriptUrl, const QString &message) const;

    void dwellShowHover(const QString &message) const;

    void dwellHide() const;

    void dwellLeave() const;

signals:
    void replaceText(const QUrl &scriptUrl, const QString &text, int lineFrom, int indexFrom, int lineTo, int indexTo);

private:
    DwellWidget *m_dwellWidget;
};

#endif //UNICOMM_CODEASSISTANT_H
