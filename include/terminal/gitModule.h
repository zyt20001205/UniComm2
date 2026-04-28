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

    Q_INVOKABLE void gitInit();

    Q_INVOKABLE void gitCommit();

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void initGit(bool status);

private:
    void processStart();

    void terminalStdout();

    void terminalStderr();

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
        Init,
        Commit
    };
};

#endif //UNICOMM_GITMODULE_H
