#ifndef UNICOMM_CMDPAGE_H
#define UNICOMM_CMDPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QProcess>

class QTextDocument;
class QQuickWidget;

class CmdPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit CmdPage(const QString &uniqueName, const QJsonObject &config);

    ~CmdPage() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void start();

    Q_INVOKABLE void terminalInput(const QString &command) const;

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

#endif //UNICOMM_CMDPAGE_H
