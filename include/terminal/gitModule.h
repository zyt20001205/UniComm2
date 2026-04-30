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

    Q_INVOKABLE void gitInit();

    Q_INVOKABLE void gitStatus() const;

    Q_INVOKABLE void gitAdd(const QUrl &documentUrl);

    Q_INVOKABLE void gitAddAll();

    Q_INVOKABLE void gitReset(const QUrl &documentUrl);

    Q_INVOKABLE void gitResetAll();

    Q_INVOKABLE void gitIgnore(const QUrl &documentUrl, bool status);

    Q_INVOKABLE void gitCommit();

signals:
    void initGit(bool status);

    void undateGit();

private:
    void terminalStdin(const QStringList &arguments) const;

    void terminalStdout();

    void terminalStderr();

    void processFinished(int exitcode);

    void parser(bool status);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QQuickItem *m_root{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    QTextDocument *m_textDocument{};
    QProcess *m_process{};

    int m_command{};

    enum GitCommand {
        Null,
        Init,
        Add,
        Reset,
        Commit
    };
};

#endif //UNICOMM_GITMODULE_H
