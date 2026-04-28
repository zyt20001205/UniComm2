#ifndef UNICOMM_GITMODULE_H
#define UNICOMM_GITMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class QProcess;
class QQuickWidget;
class QTextDocument;

class GitModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit GitModule();

    ~GitModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void terminalStdin(const QString &input) const;

    Q_INVOKABLE void gitStatus();

    Q_INVOKABLE void gitInit();

    Q_INVOKABLE void gitCommit();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void processStart();

    void terminalStdout() const;

    void terminalStderr() const;

    void parser(bool status) const;

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QQuickItem *m_root{};
    QObject *m_menu{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QTextDocument *m_textDocument{};
    QProcess *m_process{};

    int m_command{};

    enum GitCommand {
        Status,
        Init,
        Commit
    };
};

#endif //UNICOMM_GITMODULE_H
