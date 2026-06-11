#include "terminal/gitModule.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTextDocument>

#include "core/globalManager.h"

// public
GitModule::GitModule()
    : DockWidget("Git"),
      m_config(g_workspaceConfig["gitConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_branchModel(new BranchModel()),
      m_logModel(new LogModel()),
      m_textDocument(new QTextDocument()),
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
    m_widget->rootContext()->setContextProperty("branchModel", m_branchModel);
    m_widget->rootContext()->setContextProperty("logModel", m_logModel);

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
    terminalStdin(QStringList{"log", m_branch,  "-z", "--pretty=format:%H%x1e%P%x1e%ar%x1e%an%x1e%s"});
}

void GitModule::gitShow(const QString &hash) {
    m_command = Show;
    terminalStdin(QStringList{"show", hash, "-s", "--format=%s%x1e%ad%x1e%an%x1e%ae"});
}

// public: file
void GitModule::gitAdd(const QUrl &documentUrl) {
    m_command = Add;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    terminalStdin(QStringList{"add", documentPath});
}

void GitModule::gitAddAll() {
    m_command = Add;
    terminalStdin(QStringList{"add", "."});
}

void GitModule::gitReset(const QUrl &documentUrl) {
    m_command = Reset;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    terminalStdin(QStringList{"reset", documentPath});
}

void GitModule::gitResetAll() {
    m_command = Reset;
    terminalStdin(QStringList{"reset"});
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
                for (auto *item : items) {
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
            if (param.size() != 4) break;
            m_subjectLabel->setProperty("text", QString::fromLocal8Bit(param[0].trimmed()));
            m_dateLabel->setProperty("text", QString::fromLocal8Bit(param[1].trimmed()));
            m_authorLabel->setProperty("text", QString::fromLocal8Bit(param[2].trimmed()) + '<' + QString::fromLocal8Bit(param[3].trimmed()) + '>');
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
        case Add:
        case Reset: {
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
