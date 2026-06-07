#ifndef UNICOMM_GITMODULE_H
#define UNICOMM_GITMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QStandardItemModel>

class QProcess;
class QQuickWidget;
class QTextDocument;

class BranchModel;

class GitModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit GitModule();

    ~GitModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void gitInit();

    Q_INVOKABLE void gitStatus() const;

    void gitBranch();

    Q_INVOKABLE void gitSwitch(const QString &name);

    Q_INVOKABLE void gitCreate(const QString &src, const QString &dst, bool _switch);

    Q_INVOKABLE void gitRename(const QString &src, const QString &dst);

    Q_INVOKABLE void gitDelete(const QString &name);

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

    void terminalStderr() const;

    void processFinished(int exitcode);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_textArea{};
    BranchModel *m_standardItemModel{};
    QTextDocument *m_textDocument{};
    QProcess *m_process{};
    int m_command{};

    enum GitCommand {
        Null,
        Init,
        Branch,
        Switch,
        Create,
        Rename,
        Delete,
        Add,
        Reset,
        Commit
    };
};

class BranchModel final : public QStandardItemModel {
    Q_OBJECT

public:
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_GITMODULE_H
