#ifndef UNICOMM_GITMODULE_H
#define UNICOMM_GITMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>
#include <QQueue>
#include <QStandardItemModel>

class QFileSystemWatcher;
class QProcess;
class QQuickView;
class QQuickWidget;
class QTextDocument;

class BranchModel;
class LogModel;
class ShowModel;
class StatusModel;
class CommitModel;

class GitModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit GitModule();

    ~GitModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet_(const QVariantMap &objects);

    Q_INVOKABLE void branchSet(const QString &name);

    [[nodiscard]] static bool gitGet();

    Q_INVOKABLE void gitInit();

    Q_INVOKABLE void gitProxy();

    void gitWatch();

    void gitUpstream();

    Q_INVOKABLE void gitBranch();

    Q_INVOKABLE void gitSwitch(const QString &name);

    Q_INVOKABLE void gitCreate(const QString &src, const QString &dst, bool _switch);

    Q_INVOKABLE void gitRename(const QString &src, const QString &dst);

    Q_INVOKABLE void gitDelete(const QString &name);

    Q_INVOKABLE void gitLog();

    Q_INVOKABLE void gitReset(const QString &hash, int mode);

    Q_INVOKABLE void gitShowCommit(const QString &hash);

    Q_INVOKABLE void gitShowFile(const QString &hash, const QUrl &documentUrl);

    Q_INVOKABLE void gitMerge(const QString &name);

    Q_INVOKABLE void gitRebase(const QString &name);

    Q_INVOKABLE void gitAbort();

    Q_INVOKABLE void gitContinue(const QString &message);

    Q_INVOKABLE void gitDiff();

    Q_INVOKABLE void gitCommitPre();

    void gitStatus();

    Q_INVOKABLE void gitCommit(const QString &subject);

    Q_INVOKABLE void gitFetch();

    Q_INVOKABLE void gitPushPre();

    void gitAhead();

    Q_INVOKABLE void gitDiff_();

    Q_INVOKABLE void gitShowCommit_(const QString &hash);

    Q_INVOKABLE void gitPush();

    Q_INVOKABLE void gitAdd(const QUrl &documentUrl = QUrl());

    Q_INVOKABLE void gitRestore(const QUrl &documentUrl, int mode);

    Q_INVOKABLE void gitIgnore(const QUrl &documentUrl, bool status);

signals:
    void updateIndex();

    void openDocument(const QUrl &documentUrl);

    void addFinish();

    void appendBackground(int &taskId, const QString &name, const std::function<void()> &callback);

    void removeBackground(int taskId);

    void refreshBackground(int taskId, const QString &message);

private:
    void processEnqueue(int command, const QStringList &arguments);

    void processDequeue();

    void processFinished(int exitcode);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_continueDialog{};
    QObject *m_errorDialog{};
    QObject *m_canvas{};
    QObject *m_subjectLabel{};
    QObject *m_dateLabel{};
    QObject *m_authorLabel{};

    QQuickView *m_commitWindow{};
    QObject *m_commitRoot{};

    QQuickView *m_pushWindow{};
    QObject *m_pushRoot{};
    QObject *m_subjectLabel_{};
    QObject *m_dateLabel_{};
    QObject *m_authorLabel_{};

    int m_command{};
    QProcess *m_process{};
    QQueue<QVariantHash> m_queue{};
    QFileSystemWatcher *m_indexWatcher{};
    QTimer *m_indexWatcherTimer{};
    QFileSystemWatcher *m_branchWatcher{};
    QTimer *m_branchWatcherTimer{};
    QString m_current{};
    QString m_upstream{};
    int m_taskId = -1;
    BranchModel *m_branchModel{};
    LogModel *m_logModel{};
    ShowModel *m_showModel{};

    StatusModel *m_workingTreeModel{};
    StatusModel *m_indexModel{};

    CommitModel *m_commitModel{};
    ShowModel *m_showModel_{};

    struct GitCommand {
        enum {
            Null,
            Init,
            Watch,
            Branch,
            Upstream,
            Switch,
            Create,
            Rename,
            Delete,
            Log,
            Reset,
            ShowCommit,
            ShowFile,
            Merge,
            Rebase,
            Abort,
            Continue,
            Diff,
            Status,
            Commit,
            Fetch,
            Ahead,
            Diff_,
            ShowCommit_,
            Push,
            Add,
            Restore
        };
    };

    struct ResetMode {
        enum {
            Mixed,
            Soft,
            Hard,
            Merge,
            Keep
        };
    };

    struct RestoreMode {
        enum {
            Worktree,
            Staged,
            Both
        };
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
    Q_PROPERTY(QString hash READ hashGet NOTIFY hashChanged)

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString hashGet() {
        return m_hash;
    }

    void hashSet(const QString &hash) {
        if (m_hash == hash) return;
        m_hash = hash;
        emit hashChanged();
    }

signals:
    void hashChanged();

private:
    QString m_hash{};
};

class StatusModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

class CommitModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_GITMODULE_H
