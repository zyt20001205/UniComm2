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

    void start();

    Q_INVOKABLE void terminalInput(const QString &command) const;

protected:
    void closeEvent(QCloseEvent *event) override;

    QString m_processName{};

private:
    void terminalOutput() const;

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QObject *m_textField{};
    QTextDocument *m_textDocument{};
    QProcess *m_process{};
};

#endif //UNICOMM_TERMINALPAGE_H
