#ifndef UNICOMM_TERMINALPAGE_H
#define UNICOMM_TERMINALPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QProcess>

class QTextDocument;
class QQuickWidget;

class TerminalPage : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit TerminalPage(const QString &uniqueName, const QJsonObject &config);

    ~TerminalPage() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void terminalInput(const QString &command) const;

protected:
    virtual void processStart();

    void terminalOutput() const;

    QString m_processName{};
    QProcess *m_process{};

private:
    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QObject *m_textField{};
    QTextDocument *m_textDocument{};
};

#endif //UNICOMM_TERMINALPAGE_H
