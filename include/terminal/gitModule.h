#ifndef UNICOMM_GITMODULE_H
#define UNICOMM_GITMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QStandardItemModel>

class QFileSystemWatcher;
class QProcess;
class QQuickWidget;
class QTextDocument;

class BranchModel;
class LogModel;
class ShowModel;

class GitModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit GitModule();

    ~GitModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void branchSet(const QString &name);

    [[nodiscard]] static bool gitGet();

    Q_INVOKABLE void gitInit();

    void gitWatch();

    Q_INVOKABLE void gitStatus() const;

    Q_INVOKABLE void gitBranch();

    Q_INVOKABLE void gitSwitch(const QString &name);

    Q_INVOKABLE void gitCreate(const QString &src, const QString &dst, bool _switch);

    Q_INVOKABLE void gitRename(const QString &src, const QString &dst);

    Q_INVOKABLE void gitDelete(const QString &name);

    Q_INVOKABLE void gitLog();

    Q_INVOKABLE void gitReset(const QString &hash, int mode);

    Q_INVOKABLE void gitShow(const QString &hash);

    Q_INVOKABLE void gitAdd(const QUrl &documentUrl = QUrl());

    Q_INVOKABLE void gitRestore(const QUrl &documentUrl, int mode);

    Q_INVOKABLE void gitIgnore(const QUrl &documentUrl, bool status);

    Q_INVOKABLE void gitCommit();

signals:
    void updateIndex();

private:
    void terminalStdin(const QStringList &arguments) const;

    void processFinished(int exitcode);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_errorDialog{};
    QObject *m_canvas{};
    QObject *m_subjectLabel{};
    QObject *m_dateLabel{};
    QObject *m_authorLabel{};
    QProcess *m_process{};
    QFileSystemWatcher *m_indexWatcher{};
    QTimer *m_indexWatcherTimer{};
    QFileSystemWatcher *m_branchWatcher{};
    QTimer *m_branchWatcherTimer{};
    QString m_branch{};
    BranchModel *m_branchModel{};
    LogModel *m_logModel{};
    ShowModel *m_showModel{};
    int m_command{};

    enum GitCommand {
        Null,
        Init,
        Watch,
        Branch,
        Switch,
        Create,
        Rename,
        Delete,
        Log,
        Reset,
        Diff,
        Show,
        Add,
        Restore,
        Commit
    };

    enum GitResetMode {
        Mixed,
        Soft,
        Hard,
        Merge,
        Keep
    };

    enum GitRestoreMode {
        Worktree,
        Staged,
        Both
    };
};

class BranchModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

class LogModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

class ShowModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_GITMODULE_H
