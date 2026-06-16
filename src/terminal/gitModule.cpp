#include "terminal/gitModule.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QTimer>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QTextDocument>

#include "core/globalManager.h"
#include "util/uniCast.h"

// public
GitModule::GitModule()
    : DockWidget("Git"),
      m_config(g_workspaceConfig["gitConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_commitWindow(new QQuickView()),
      m_pushWindow(new QQuickView()),
      m_process(new QProcess(this)),
      m_indexWatcher(new QFileSystemWatcher(this)),
      m_indexWatcherTimer(new QTimer(this)),
      m_branchWatcher(new QFileSystemWatcher(this)),
      m_branchWatcherTimer(new QTimer(this)),
      m_branchModel(new BranchModel(this)),
      m_logModel(new LogModel(this)),
      m_showModel(new ShowModel(this)),
      m_workingTreeModel(new StatusModel(this)),
      m_indexModel(new StatusModel(this)),
      m_commitModel(new CommitModel(this)),
      m_showModel_(new ShowModel(this)) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);

    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::finished, this, [this](const int exitcode) { processFinished(exitcode); });

    connect(m_indexWatcher, &QFileSystemWatcher::fileChanged, this, [this] { m_indexWatcherTimer->start(); });
    m_indexWatcherTimer->setSingleShot(true);
    m_indexWatcherTimer->setInterval(100);
    connect(m_indexWatcherTimer, &QTimer::timeout, this, &GitModule::updateIndex);
    connect(m_indexWatcherTimer, &QTimer::timeout, this, &GitModule::gitStatus);

    connect(m_branchWatcher, &QFileSystemWatcher::fileChanged, this, [this] { m_branchWatcherTimer->start(); });
    m_branchWatcherTimer->setSingleShot(true);
    m_branchWatcherTimer->setInterval(100);
    connect(m_branchWatcherTimer, &QTimer::timeout, this, &GitModule::gitBranch);
}

GitModule::~GitModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void GitModule::propertySet(const QVariantHash &objects) {
    m_errorDialog = qvariant_cast<QObject *>(objects["gitModuleErrorDialog"]);

    m_widget->rootContext()->setContextProperty("gitModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_widget->rootContext()->setContextProperty("branchMenu", objects["gitModuleBranchMenu"]);
    m_widget->rootContext()->setContextProperty("logMenu", objects["gitModuleLogMenu"]);
    m_widget->rootContext()->setContextProperty("branchModel", m_branchModel);
    m_widget->rootContext()->setContextProperty("logModel", m_logModel);
    m_widget->rootContext()->setContextProperty("showModel", m_showModel);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/gitModule.qml"));
    m_root = m_widget->rootObject();
    if (g_globalManager->gitGet()) gitWatch();

    // commit window
    m_commitWindow->setTitle(tr("Git Commit"));
    m_commitWindow->setTransientParent(g_mainWindow->windowHandle());

    m_commitWindow->rootContext()->setContextProperty("gitModule", this);
    m_commitWindow->rootContext()->setContextProperty("global", objects["global"]);
    m_commitWindow->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_commitWindow->rootContext()->setContextProperty("workingTreeModel", m_workingTreeModel);
    m_commitWindow->rootContext()->setContextProperty("indexModel", m_indexModel);

    m_commitWindow->setResizeMode(QQuickView::SizeRootObjectToView);
    m_commitWindow->setSource(QUrl("qrc:/qml/terminal/gitCommitWindow.qml"));
    m_commitRoot = m_commitWindow->rootObject();

    // push window
    m_pushWindow->setTitle(tr("Git Push"));
    m_pushWindow->setTransientParent(g_mainWindow->windowHandle());

    m_pushWindow->rootContext()->setContextProperty("gitModule", this);
    m_pushWindow->rootContext()->setContextProperty("global", objects["global"]);
    m_pushWindow->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_pushWindow->rootContext()->setContextProperty("commitModel", m_commitModel);
    m_pushWindow->rootContext()->setContextProperty("showModel", m_showModel_);

    m_pushWindow->setResizeMode(QQuickView::SizeRootObjectToView);
    m_pushWindow->setSource(QUrl("qrc:/qml/terminal/gitPushWindow.qml"));
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_canvas = qvariant_cast<QObject *>(objects["canvas"]);
    m_subjectLabel = qvariant_cast<QObject *>(objects["subjectLabel"]);
    m_dateLabel = qvariant_cast<QObject *>(objects["dateLabel"]);
    m_authorLabel = qvariant_cast<QObject *>(objects["authorLabel"]);
}

void GitModule::propertyGet_(const QVariantMap &objects) {
    m_subjectLabel_ = qvariant_cast<QObject *>(objects["subjectLabel"]);
    m_dateLabel_ = qvariant_cast<QObject *>(objects["dateLabel"]);
    m_authorLabel_ = qvariant_cast<QObject *>(objects["authorLabel"]);
}

void GitModule::branchSet(const QString &name) {
    if (m_branch == name) return;
    m_branch = name;
    gitLog();
}

bool GitModule::gitGet() {
    QProcess process{};
    process.start("git", {"--version"});
    process.waitForFinished();
    if (process.exitCode() == 0) {
        process.setWorkingDirectory(g_workspaceUrl.toLocalFile());
        process.start("git", {"status"});
        process.waitForFinished();
        if (process.exitCode() == 0) {
            return true;
        }
    }
    return false;
}

void GitModule::gitInit() {
    processEnqueue(Init, QStringList{"init"});
}

void GitModule::gitWatch() {
    const auto &files = m_branchWatcher->files();
    if (!files.isEmpty()) m_branchWatcher->removePaths(files);
    processEnqueue(Watch, QStringList{"rev-parse", "--absolute-git-dir"});
}

// public: branch
void GitModule::gitBranch() {
    processEnqueue(Branch, QStringList{"branch", "-avv"});
}

void GitModule::gitSwitch(const QString &name) {
    processEnqueue(Switch, QStringList{"switch", name});
}

void GitModule::gitCreate(const QString &src, const QString &dst, const bool _switch) {
    QStringList arguments{};
    if (_switch) arguments = QStringList{"switch", "-c", dst, src};
    else arguments = QStringList{"branch", dst, src};
    processEnqueue(Create, arguments);
}

void GitModule::gitRename(const QString &src, const QString &dst) {
    processEnqueue(Rename, QStringList{"branch", "-m", src, dst});
}

void GitModule::gitDelete(const QString &name) {
    processEnqueue(Delete, QStringList{"branch", "-D", name});
}

void GitModule::gitLog() {
    if (m_branch.isEmpty()) return;
    processEnqueue(Log, QStringList{"log", m_branch, "-z", "--pretty=format:%h%x1e%p%x1e%ar%x1e%an%x1e%s"});
}

void GitModule::gitReset(const QString &hash, const int mode) {
    QString _mode{};
    switch (mode) {
        case Mixed: _mode = "--mixed";
            break;
        case Soft: _mode = "--soft";
            break;
        case Hard: _mode = "--hard";
            break;
        case Merge: _mode = "--merge";
            break;
        case Keep: _mode = "--keep";
            break;
        default: return;
    }
    processEnqueue(Reset, QStringList{"reset", hash, _mode});
}

void GitModule::gitShow(const QString &hash) {
    processEnqueue(Show, QStringList{"show", hash, "--format=%h%x1e%s%x1e%ad%x1e%an%x1e%ae%x1e", "--name-status"});
}

void GitModule::gitDiff(const QString &hash, const QUrl &documentUrl) {
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    processEnqueue(Diff, QStringList{"show", hash, "--format=", relativePath});
}

void GitModule::gitFetch() {
    processEnqueue(Fetch, QStringList{"fetch"});
}

// public: commit
void GitModule::gitCommitPre() {
    QMetaObject::invokeMethod(m_commitRoot, "reset");
    m_commitWindow->resize(1080, 720);
    m_commitWindow->show();
    gitStatus();
}

void GitModule::gitStatus() {
    processEnqueue(Status, QStringList{"status", "-uall", "--porcelain"});
}

void GitModule::gitCommit(const QString &subject) {
    m_commitWindow->close();
    processEnqueue(Commit, QStringList{"commit", "-m", subject});
}

// public: push
void GitModule::gitPushPre() {
    m_pushWindow->resize(1080, 720);
    m_pushWindow->show();
    gitUpstream();
}

void GitModule::gitUpstream() {
    processEnqueue(Upstream, QStringList{"rev-parse", "--abbrev-ref", "@{upstream}"});
}

void GitModule::gitAhead() {
    processEnqueue(Ahead, QStringList{"log", "@{upstream}..HEAD", "-z", "--pretty=format:%h%x1e%s"});
}

void GitModule::gitDiff_() {
    processEnqueue(Diff_, QStringList{"diff", "--name-status", "@{upstream}..HEAD"});
}

void GitModule::gitShow_(const QString &hash) {
    processEnqueue(Show_, QStringList{"show", hash, "--format=%h%x1e%s%x1e%ad%x1e%an%x1e%ae%x1e", "--name-status"});
}

void GitModule::gitPush() {
    processEnqueue(Push, QStringList{"push"});
}

// public: file
void GitModule::gitAdd(const QUrl &documentUrl) {
    QStringList arguments{};
    if (documentUrl.isEmpty()) {
        arguments = QStringList{"add", "."};
    } else {
        const auto documentPath = documentUrl.toLocalFile();
        const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
        const auto relativePath = workspaceDir.relativeFilePath(documentPath);
        arguments = QStringList{"add", documentPath};
    }
    processEnqueue(Add, arguments);
}

void GitModule::gitRestore(const QUrl &documentUrl, const int mode) {
    QStringList arguments{"restore"};
    switch (mode) {
        case Worktree: arguments << "--worktree";
            break;
        case Staged: arguments << "--staged";
            break;
        case Both: arguments << "--worktree" << "--staged";
            break;
        default: return;
    }
    if (documentUrl.isEmpty()) {
        arguments << ".";
    } else {
        const auto documentPath = documentUrl.toLocalFile();
        const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
        const auto relativePath = workspaceDir.relativeFilePath(documentPath);
        arguments << documentPath;
    }
    processEnqueue(Restore, arguments);
}

void GitModule::gitIgnore(const QUrl &documentUrl, const bool status) {
    // check file and open
    const auto gitignorePath = QDir(g_workspaceUrl.toLocalFile()).filePath(".gitignore");
    auto gitignoreFile = QFile(gitignorePath);
    if (!gitignoreFile.exists()) {
        if (!gitignoreFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        gitignoreFile.close();
    }
    // read to string list
    QStringList gitignoreList{};
    if (!gitignoreFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&gitignoreFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        gitignoreList.append(line);
    }
    gitignoreFile.close();
    // ready to add / remove
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = '/' + workspaceDir.relativeFilePath(documentPath);
    if (status) {
        bool inserted = false;
        for (int i = 0; i < gitignoreList.size(); ++i) {
            if (gitignoreList[i].startsWith('#') || gitignoreList[i].trimmed().isEmpty()) continue;
            if (relativePath == gitignoreList[i]) break;
            if (relativePath < gitignoreList[i]) {
                gitignoreList.insert(i, relativePath);
                inserted = true;
                break;
            }
        }
        if (!inserted) gitignoreList.append(relativePath);
    } else {
        gitignoreList.removeAll(relativePath);
    }
    // write back
    if (!gitignoreFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&gitignoreFile);
    for (const QString &line: gitignoreList) {
        out << line << "\n";
    }
    gitignoreFile.close();

    emit updateIndex();
}

// private
void GitModule::processEnqueue(int command, const QStringList &arguments) {
    m_queue.enqueue(QVariantHash{
        {"command", command},
        {"arguments", arguments},
    });
    if (m_process->state() == QProcess::NotRunning) processDequeue();
}

void GitModule::processDequeue() {
    if (m_process->state() != QProcess::NotRunning) return;
    if (m_queue.isEmpty()) return;
    const auto &session = m_queue.dequeue();
    m_command = session["command"].toInt();
    m_process->start("git", session["arguments"].toStringList());
}

void GitModule::processFinished(const int exitcode) {
    const auto output = m_process->readAllStandardOutput();
    const auto error = m_process->readAllStandardError();
    const auto command = m_command;
    m_command = Null;

    if (exitcode != 0) {
        m_errorDialog->setProperty("title", "Git command failed");
        m_errorDialog->setProperty("text", QString::fromLocal8Bit(error).trimmed());
        QMetaObject::invokeMethod(m_errorDialog, "open");
        processDequeue();
        return;
    }

    switch (command) {
        case Watch: {
            const auto &gitPath = QString::fromLocal8Bit(output).trimmed();
            const auto &gitDir = QDir(gitPath);

            const auto &indexPath = gitDir.filePath("index");
            if (QFileInfo::exists(indexPath)) m_indexWatcher->addPath(indexPath);

            const auto &headPath = gitDir.filePath("HEAD");
            const auto &refsPath = gitDir.filePath("refs");
            if (QFileInfo::exists(headPath)) m_branchWatcher->addPath(headPath);
            if (QFileInfo::exists(refsPath)) {
                auto iterator = QDirIterator(refsPath, QDir::Files, QDirIterator::Subdirectories);
                while (iterator.hasNext()) m_branchWatcher->addPath(iterator.next());
            }
        }
        break;
        case Branch: {
            m_branchModel->clear();
            auto *localItem = new QStandardItem(tr("Local")); // NOLINT
            localItem->setData("local", Qt::UserRole + 1);
            m_branchModel->appendRow(localItem);
            QStandardItem *remoteItem = nullptr;
            for (const auto &value: QString::fromLocal8Bit(output).split('\n')) {
                QString branch{};
                QString type = "untracked";
                if (value.startsWith('*')) {
                    type = "current";
                }
                branch = value.mid(2);
                const auto param = branch.split(' ', Qt::SkipEmptyParts);
                if (param.size() < 2 || param[1] == "->") continue;
                const auto &name = param[0];
                const auto &hash = param[1];
                const auto &commit = QStringList(param.mid(2)).join(' ');
                auto *item = new QStandardItem(name.startsWith("remotes/") ? name.mid(8) : name); // NOLINT
                item->setData(type, Qt::UserRole + 1);
                item->setData(hash, Qt::UserRole + 2);
                item->setData(commit, Qt::UserRole + 3);
                // remote
                if (name.startsWith("remotes/")) {
                    if (remoteItem == nullptr) {
                        remoteItem = new QStandardItem(tr("Remote")); // NOLINT
                        remoteItem->setData("remote", Qt::UserRole + 1);
                        m_branchModel->appendRow(remoteItem);
                    }
                    remoteItem->appendRow(item);
                }
                // local
                else {
                    if (name == "master") localItem->insertRow(0, item);
                    else localItem->appendRow(item);
                }
            }
            QMetaObject::invokeMethod(m_root, "branchExpand");
        }
        break;
        case Log: {
            m_logModel->clear();
            QHash<QString, QPoint> nodeHash{};
            QList<QStringList> parentRows{};
            QStringList lanes{};
            int laneCount = 0;
            for (const auto &value: output.split('\0')) {
                const auto param = value.split('\x1e');
                if (param.size() != 5) continue;

                const QString &hash = QString::fromUtf8(param[0]);
                const QStringList parents = QString::fromUtf8(param[1]).split(' ', Qt::SkipEmptyParts);
                int lane = static_cast<int>(lanes.indexOf(hash));
                if (lane < 0) {
                    lane = 0;
                    lanes.insert(0, hash);
                }
                const int index = m_logModel->rowCount();
                const QPoint nodePos(lane, index);
                nodeHash.insert(hash, nodePos);
                parentRows.append(parents);

                lanes.removeAt(lane);
                int insertLane = lane;
                for (const QString &parentHash: parents) {
                    if (lanes.contains(parentHash)) continue;
                    lanes.insert(insertLane++, parentHash);
                }
                laneCount = qMax(laneCount, static_cast<int>(lanes.size()));

                const auto &date = QString::fromUtf8(param[2]);
                const auto &author = QString::fromUtf8(param[3]);
                const auto subject = QString::fromUtf8(param[4]);
                const auto &nodeItem = new QStandardItem(); // NOLINT
                const QList items{new QStandardItem(date), new QStandardItem(author), new QStandardItem(subject), nodeItem};
                for (auto *item: items) {
                    item->setData(hash, Qt::UserRole + 1);
                }
                nodeItem->setData(nodePos, Qt::UserRole + 2);
                m_logModel->appendRow(items);
            }

            for (int row = 0; row < parentRows.size(); ++row) {
                QVariantList parentPositions{};
                for (const QString &parentHash: parentRows[row]) {
                    const auto parent = nodeHash.constFind(parentHash);
                    if (parent != nodeHash.cend()) parentPositions.append(parent.value());
                }
                m_logModel->item(row, 3)->setData(parentPositions, Qt::UserRole + 3);
            }
            m_canvas->setProperty("laneCount", qMax(laneCount, 1));
        }
        break;
        case Show: {
            const auto param = output.split('\x1e');
            if (param.size() != 6) break;
            const auto &hash = QString::fromLocal8Bit(param[0].trimmed());
            m_subjectLabel->setProperty("text", '(' + hash + ')' + QString::fromLocal8Bit(param[1].trimmed()));
            m_dateLabel->setProperty("text", QString::fromLocal8Bit(param[2].trimmed()));
            m_authorLabel->setProperty("text", QString::fromLocal8Bit(param[3].trimmed()) + '<' + QString::fromLocal8Bit(param[4].trimmed()) + '>');

            m_showModel->clear();
            m_showModel->hashSet(QString());
            m_showModel->hashSet(hash);
            QHash<QString, QStandardItem *> roots{};
            const auto &changes = QString::fromUtf8(param[5]).split('\n', Qt::SkipEmptyParts);
            for (const auto &value: changes) {
                const auto change = value.split('\t');
                if (change.size() < 2) continue;
                const auto &status = g_gitStatus[change[0].front()];
                QString path1{};
                QString path2{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    path1 = change[1];
                    path2 = change[2];
                } else {
                    path2 = change[1];
                }
                const auto &path = path2.split('/');
                const auto &documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(path2);
                const auto &documentUrl = QUrl::fromLocalFile(documentPath);
                QString display{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    display = QString("%1 -> %2 (%3%)").arg(
                        QUrl::fromLocalFile(QDir(g_workspaceUrl.toLocalFile()).filePath(path1)).fileName(),
                        documentUrl.fileName(),
                        change[0].mid(1)
                    );
                } else {
                    display = documentUrl.fileName();
                }
                const auto &source = uni_cast<QFileIcon>(documentUrl);

                QStandardItem *rootItem{};
                QString rootPath{};
                for (int i = 0; i < path.size() - 1; ++i) {
                    if (!rootPath.isEmpty()) rootPath += '/';
                    rootPath += path[i];

                    auto *_rootItem = roots.value(rootPath);
                    if (!_rootItem) {
                        _rootItem = new QStandardItem(path[i]); // NOLINT
                        _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                        if (rootItem) rootItem->appendRow(_rootItem);
                        else m_showModel->appendRow(_rootItem);
                        roots.insert(rootPath, _rootItem);
                    }
                    rootItem = _rootItem;
                }

                auto *item = new QStandardItem(display); // NOLINT
                item->setData(source.value, Qt::DecorationRole);
                item->setData(documentUrl, Qt::UserRole + 1);
                item->setData(status, Qt::UserRole + 2);
                if (rootItem) rootItem->appendRow(item);
                else m_showModel->appendRow(item);
            }
        }
        break;
        case Diff: {
            qDebug() << output;
        }
        break;
        case Status: {
            m_workingTreeModel->clear();
            m_indexModel->clear();
            QHash<QString, QStandardItem *> workingTreeRoots{};
            QHash<QString, QStandardItem *> indexRoots{};

            const auto changes = output.split('\n');
            for (const auto &value: changes) {
                if (value.size() < 4) continue;
                const auto &change = QString::fromUtf8(value);

                const auto indexStatus = g_gitStatus[change.at(0)];
                const auto workingTreeStatus = g_gitStatus[change.at(1)];
                auto path = change.mid(3).trimmed();
                if (indexStatus == 'R' || indexStatus == 'C' || workingTreeStatus == 'R' || workingTreeStatus == 'C') path = path.section(" -> ", 1);
                const auto &documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(path);
                const auto &documentUrl = QUrl::fromLocalFile(documentPath);
                const auto &display = documentUrl.fileName();
                const auto &source = uni_cast<QFileIcon>(documentUrl);
                const auto &pathList = path.split('/');

                // append working tree model
                if (workingTreeStatus != GitStatus::Unmodified) {
                    QStandardItem *rootItem{};
                    QString rootPath{};
                    for (int i = 0; i < pathList.size() - 1; ++i) {
                        if (!rootPath.isEmpty()) rootPath += '/';
                        rootPath += pathList[i];

                        auto *_rootItem = workingTreeRoots.value(rootPath);
                        if (!_rootItem) {
                            _rootItem = new QStandardItem(pathList[i]); // NOLINT
                            _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                            _rootItem->setData(QUrl::fromLocalFile(QDir(g_workspaceUrl.toLocalFile()).filePath(rootPath)), Qt::UserRole + 1);

                            if (rootItem) rootItem->appendRow(_rootItem);
                            else m_workingTreeModel->appendRow(_rootItem);
                            workingTreeRoots.insert(rootPath, _rootItem);
                        }
                        rootItem = _rootItem;
                    }

                    auto *item = new QStandardItem(display); // NOLINT
                    item->setData(source.value, Qt::DecorationRole);
                    item->setData(documentUrl, Qt::UserRole + 1);
                    item->setData(workingTreeStatus, Qt::UserRole + 2);
                    if (rootItem) rootItem->appendRow(item);
                    else m_workingTreeModel->appendRow(item);

                    // skip index model if not added
                    if (workingTreeStatus == GitStatus::Untracked) continue;
                }

                // append index model
                if (indexStatus != GitStatus::Unmodified) {
                    QStandardItem *rootItem{};
                    QString rootPath{};
                    for (int i = 0; i < pathList.size() - 1; ++i) {
                        if (!rootPath.isEmpty()) rootPath += '/';
                        rootPath += pathList[i];

                        auto *_rootItem = indexRoots.value(rootPath);
                        if (!_rootItem) {
                            _rootItem = new QStandardItem(pathList[i]); // NOLINT
                            _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                            _rootItem->setData(QUrl::fromLocalFile(QDir(g_workspaceUrl.toLocalFile()).filePath(rootPath)), Qt::UserRole + 1);

                            if (rootItem) rootItem->appendRow(_rootItem);
                            else m_indexModel->appendRow(_rootItem);
                            indexRoots.insert(rootPath, _rootItem);
                        }
                        rootItem = _rootItem;
                    }

                    auto *item = new QStandardItem(display); // NOLINT
                    item->setData(source.value, Qt::DecorationRole);
                    item->setData(documentUrl, Qt::UserRole + 1);
                    item->setData(indexStatus, Qt::UserRole + 2);
                    if (rootItem) rootItem->appendRow(item);
                    else m_indexModel->appendRow(item);
                }
            }
            QMetaObject::invokeMethod(m_commitRoot, "workingTreeExpand");
            QMetaObject::invokeMethod(m_commitRoot, "indexExpand");
        }
        break;
        case Upstream: {
            m_commitModel->clear();
            m_commitModel->appendRow(new QStandardItem("->" + QString::fromLocal8Bit(output)));
        }
        break;
        case Ahead: {
            if (m_commitModel->rowCount() <= 0) break;
            auto *root = m_commitModel->item(0, 0);
            for (const auto &value: output.split('\0')) {
                const auto param = value.split('\x1e');
                if (param.size() < 2) continue;
                const auto &hash = param[0];
                const auto &subject = param[1];
                auto *item = new QStandardItem(subject); // NOLINT
                item->setData(hash, Qt::UserRole + 1);
                root->appendRow(item);
            }
        }
        break;
        case Diff_: {
            m_showModel_->clear();
            QHash<QString, QStandardItem *> roots{};
            const auto &changes = QString::fromUtf8(output).split('\n', Qt::SkipEmptyParts);
            for (const auto &value: changes) {
                const auto change = value.split('\t');
                if (change.size() < 2) continue;
                const auto &status = g_gitStatus[change[0].front()];
                QString path1{};
                QString path2{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    path1 = change[1];
                    path2 = change[2];
                } else {
                    path2 = change[1];
                }
                const auto &path = path2.split('/');
                const auto &documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(path2);
                const auto &documentUrl = QUrl::fromLocalFile(documentPath);
                QString display{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    display = QString("%1 -> %2 (%3%)").arg(
                        QUrl::fromLocalFile(QDir(g_workspaceUrl.toLocalFile()).filePath(path1)).fileName(),
                        documentUrl.fileName(),
                        change[0].mid(1)
                    );
                } else {
                    display = documentUrl.fileName();
                }
                const auto &source = uni_cast<QFileIcon>(documentUrl);

                QStandardItem *rootItem{};
                QString rootPath{};
                for (int i = 0; i < path.size() - 1; ++i) {
                    if (!rootPath.isEmpty()) rootPath += '/';
                    rootPath += path[i];

                    auto *_rootItem = roots.value(rootPath);
                    if (!_rootItem) {
                        _rootItem = new QStandardItem(path[i]); // NOLINT
                        _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                        if (rootItem) rootItem->appendRow(_rootItem);
                        else m_showModel_->appendRow(_rootItem);
                        roots.insert(rootPath, _rootItem);
                    }
                    rootItem = _rootItem;
                }

                auto *item = new QStandardItem(display); // NOLINT
                item->setData(source.value, Qt::DecorationRole);
                item->setData(documentUrl, Qt::UserRole + 1);
                item->setData(status, Qt::UserRole + 2);
                if (rootItem) rootItem->appendRow(item);
                else m_showModel_->appendRow(item);
            }
        }
        break;
        case Show_: {
            const auto param = output.split('\x1e');
            if (param.size() != 6) break;
            m_subjectLabel_->setProperty("text", '(' + QString::fromLocal8Bit(param[0].trimmed()) + ')' + QString::fromLocal8Bit(param[1].trimmed()));
            m_dateLabel_->setProperty("text", QString::fromLocal8Bit(param[2].trimmed()));
            m_authorLabel_->setProperty("text", QString::fromLocal8Bit(param[3].trimmed()) + '<' + QString::fromLocal8Bit(param[4].trimmed()) + '>');

            m_showModel_->clear();
            QHash<QString, QStandardItem *> roots{};
            const auto &changes = QString::fromUtf8(param[5]).split('\n', Qt::SkipEmptyParts);
            for (const auto &value: changes) {
                const auto change = value.split('\t');
                if (change.size() < 2) continue;
                const auto &status = g_gitStatus[change[0].front()];
                QString path1{};
                QString path2{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    path1 = change[1];
                    path2 = change[2];
                } else {
                    path2 = change[1];
                }
                const auto &path = path2.split('/');
                const auto &documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(path2);
                const auto &documentUrl = QUrl::fromLocalFile(documentPath);
                QString display{};
                if (status == GitStatus::Renamed || status == GitStatus::Copied) {
                    display = QString("%1 -> %2 (%3%)").arg(
                        QUrl::fromLocalFile(QDir(g_workspaceUrl.toLocalFile()).filePath(path1)).fileName(),
                        documentUrl.fileName(),
                        change[0].mid(1)
                    );
                } else {
                    display = documentUrl.fileName();
                }
                const auto &source = uni_cast<QFileIcon>(documentUrl);

                QStandardItem *rootItem{};
                QString rootPath{};
                for (int i = 0; i < path.size() - 1; ++i) {
                    if (!rootPath.isEmpty()) rootPath += '/';
                    rootPath += path[i];

                    auto *_rootItem = roots.value(rootPath);
                    if (!_rootItem) {
                        _rootItem = new QStandardItem(path[i]); // NOLINT
                        _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                        if (rootItem) rootItem->appendRow(_rootItem);
                        else m_showModel_->appendRow(_rootItem);
                        roots.insert(rootPath, _rootItem);
                    }
                    rootItem = _rootItem;
                }

                auto *item = new QStandardItem(display); // NOLINT
                item->setData(source.value, Qt::DecorationRole);
                item->setData(documentUrl, Qt::UserRole + 1);
                item->setData(status, Qt::UserRole + 2);
                if (rootItem) rootItem->appendRow(item);
                else m_showModel_->appendRow(item);
            }
        }
        break;
        default: break;
    }

    // state machine
    switch (command) {
        case Init: {
            g_globalManager->gitSet();
            emit updateIndex();
            gitWatch();
        }
        break;
        case Watch: {
            gitBranch();
        }
        break;
        case Branch: {
            gitLog();
        }
        break;
        case Create:
        case Rename:
        case Delete: {
            gitWatch();
        }
        break;
        case Log: {
            if (m_logModel->rowCount() > 0) {
                const auto hash = m_logModel->item(0, 0)->data(Qt::UserRole + 1).toString();
                gitShow(hash);
            }
        }
        break;
        case Upstream: {
            gitAhead();
        }
        break;
        case Ahead: {
            gitDiff_();
        }
        break;
        default: break;
    }

    processDequeue();
}

// public
QHash<int, QByteArray> BranchModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "type";
    roles[Qt::UserRole + 2] = "hash";
    roles[Qt::UserRole + 3] = "commit";
    return roles;
}

// public
QHash<int, QByteArray> LogModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "hash";
    roles[Qt::UserRole + 2] = "pos";
    roles[Qt::UserRole + 3] = "parent";
    return roles;
}

// public
QHash<int, QByteArray> ShowModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "documentUrl";
    roles[Qt::UserRole + 2] = "status";
    return roles;
}

// public
QHash<int, QByteArray> StatusModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "documentUrl";
    roles[Qt::UserRole + 2] = "status";
    return roles;
}

// public
QHash<int, QByteArray> CommitModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "hash";
    return roles;
}
