#include "terminal/gitModule.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTextDocument>

#include "core/globalManager.h"
#include "util/uniCast.h"

// public
GitModule::GitModule()
    : DockWidget("Git"),
      m_config(g_workspaceConfig["gitConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_branchModel(new BranchModel(this)),
      m_logModel(new LogModel(this)),
      m_showModel(new ShowModel(this)),
      m_process(new QProcess(this)) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);

    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyReadStandardError, this, &GitModule::terminalStderr);
    connect(m_process, &QProcess::finished, this, [this](const int exitcode) { processFinished(exitcode); });
}

GitModule::~GitModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void GitModule::propertySet(const QVariantHash &objects) {
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
    if (g_globalManager->gitGet()) gitBranch();
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_canvas = qvariant_cast<QObject *>(objects["canvas"]);
    m_subjectLabel = qvariant_cast<QObject *>(objects["subjectLabel"]);
    m_dateLabel = qvariant_cast<QObject *>(objects["dateLabel"]);
    m_authorLabel = qvariant_cast<QObject *>(objects["authorLabel"]);
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
    m_command = Init;
    terminalStdin(QStringList{"init"});
}

void GitModule::gitStatus() const {
    terminalStdin(QStringList{"status", "--porcelain"});
}

// public: branch
void GitModule::gitBranch() {
    m_command = Branch;
    terminalStdin(QStringList{"branch", "-av"});
}

void GitModule::gitSwitch(const QString &name) {
    m_command = Switch;
    terminalStdin(QStringList{"switch", name});
}

void GitModule::gitCreate(const QString &src, const QString &dst, const bool _switch) {
    m_command = Create;
    if (_switch) terminalStdin(QStringList{"switch", "-c", dst, src});
    else terminalStdin(QStringList{"branch", dst, src});
}

void GitModule::gitRename(const QString &src, const QString &dst) {
    m_command = Rename;
    terminalStdin(QStringList{"branch", "-m", src, dst});
}

void GitModule::gitDelete(const QString &name) {
    m_command = Delete;
    terminalStdin(QStringList{"branch", "-D", name});
}

void GitModule::gitLog() {
    if (m_branch.isEmpty()) return;
    m_command = Log;
    terminalStdin(QStringList{"log", m_branch, "-z", "--pretty=format:%h%x1e%p%x1e%ar%x1e%an%x1e%s"});
}

void GitModule::gitReset(const QString &hash, const int mode) {
    QString _mode{};
    switch (mode) {
        case 0: _mode = "--mixed";
            break;
        case 1: _mode = "--soft";
            break;
        case 2: _mode = "--hard";
            break;
        case 3: _mode = "--merge";
            break;
        case 4: _mode = "--keep";
            break;
        default: return;
    }
    m_command = Reset;
    terminalStdin(QStringList{"reset", hash, _mode});
}

void GitModule::gitShow(const QString &hash) {
    m_command = Show;
    terminalStdin(QStringList{"show", hash, "--format=%h%x1e%s%x1e%ad%x1e%an%x1e%ae%x1e", "--name-status"});
}

// public: file
// void GitModule::gitAdd(const QUrl &documentUrl) {
//     m_command = Add;
//     const auto documentPath = documentUrl.toLocalFile();
//     const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
//     const auto relativePath = workspaceDir.relativeFilePath(documentPath);
//     terminalStdin(QStringList{"add", documentPath});
// }

// void GitModule::gitAddAll() {
//     m_command = Add;
//     terminalStdin(QStringList{"add", "."});
// }

// void GitModule::gitReset(const QUrl &documentUrl) {
//     m_command = Reset;
//     const auto documentPath = documentUrl.toLocalFile();
//     const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
//     const auto relativePath = workspaceDir.relativeFilePath(documentPath);
//     terminalStdin(QStringList{"reset", documentPath});
// }

// void GitModule::gitResetAll() {
//     m_command = Reset;
//     terminalStdin(QStringList{"reset"});
// }

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
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
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

    emit undateGit();
}

void GitModule::gitCommit() {
    m_command = Commit;
    terminalStdin(QStringList{"commit", "-m", "test"});
}

// private
void GitModule::terminalStdin(const QStringList &arguments) const {
    m_process->start("git", arguments);
}

void GitModule::terminalStderr() const {
    const auto error = QString::fromLocal8Bit(m_process->readAllStandardError());
    qDebug() << error;
}

void GitModule::processFinished(const int exitcode) {
    const auto output = m_process->readAllStandardOutput();
    const auto command = m_command;
    switch (command) {
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
            m_subjectLabel->setProperty("text", '(' + QString::fromLocal8Bit(param[0].trimmed()) + ')' + QString::fromLocal8Bit(param[1].trimmed()));
            m_dateLabel->setProperty("text", QString::fromLocal8Bit(param[2].trimmed()));
            m_authorLabel->setProperty("text", QString::fromLocal8Bit(param[3].trimmed()) + '<' + QString::fromLocal8Bit(param[4].trimmed()) + '>');

            m_showModel->clear();
            QHash<QString, QStandardItem *> rootItems{};
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
                const auto documentPath = QDir(g_workspaceUrl.toLocalFile()).filePath(path2);
                const auto documentUrl = QUrl::fromLocalFile(documentPath);
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
                const auto source = uni_cast<QFileIcon>(documentUrl);

                QStandardItem *rootItem{};
                QString rootPath{};
                for (int i = 0; i < path.size() - 1; ++i) {
                    if (!rootPath.isEmpty()) rootPath += '/';
                    rootPath += path[i];

                    auto *_rootItem = rootItems.value(rootPath);
                    if (!_rootItem) {
                        _rootItem = new QStandardItem(path[i]); // NOLINT
                        _rootItem->setData(QUrl("qrc:/icon/fileTypeFolder.svg"), Qt::DecorationRole);
                        if (rootItem) rootItem->appendRow(_rootItem);
                        else m_showModel->appendRow(_rootItem);
                        rootItems.insert(rootPath, _rootItem);
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
        default: break;
    }

    m_command = Null;
    switch (command) {
        case Init: {
            g_globalManager->gitSet();
            emit undateGit();
        }
        break;
        case Branch: {
            gitLog();
        }
        break;
        case Switch:
        case Create:
        case Rename:
        case Delete: {
            gitBranch();
        }
        break;
        case Log: {
            if (m_logModel->rowCount() > 0) {
                const auto hash = m_logModel->item(0, 0)->data(Qt::UserRole + 1).toString();
                gitShow(hash);
            }
        }
        break;
        case Reset: {
            gitBranch();
        }
        break;
        case Add:
        case Checkout: {
            emit undateGit();
        }
        break;
        case Commit: {
            gitBranch();
            emit undateGit();
        }
        break;
        default: break;
    }
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
