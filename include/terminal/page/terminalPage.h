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

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE virtual void terminalInput(const QString &input) const;

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual void processStart();

    void terminalOutput() const;

    QString m_name{};
    QStringList m_arguments{};
    QProcess *m_process{};

private:
    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QTextDocument *m_textDocument{};
};

#endif //UNICOMM_TERMINALPAGE_H
