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
      m_standardItemModel(new BranchModel()),
      m_textDocument(new QTextDocument()),
      m_process(new QProcess(this)) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);

    m_process->setWorkingDirectory(g_workspaceUrl.toLocalFile());
    connect(m_process, &QProcess::readyReadStandardOutput, this, &GitModule::terminalStdout);
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
    m_widget->rootContext()->setContextProperty("branchMenu", objects["gitModuleBranchMenu"]);
    m_widget->rootContext()->setContextProperty("standardItemModel", m_standardItemModel);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/gitModule.qml"));
    m_root = m_widget->rootObject();
    if (g_globalManager->gitGet()) gitBranch();
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
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
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, {"git init"}));
    terminalStdin(QStringList{"init"});
}

void GitModule::gitStatus() const {
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git status --porcelain"));
    terminalStdin(QStringList{"status", "--porcelain"});
}

// public: branch
void GitModule::gitBranch() {
    m_command = Branch;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git branch -av"));
    terminalStdin(QStringList{"branch", "-av"});
}

void GitModule::gitSwitch(const QString &name) {
    m_command = Switch;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git switch " + name));
    terminalStdin(QStringList{"switch", name});
}

void GitModule::gitCreate(const QString &src, const QString &dst, const bool _switch) {
    m_command = Create;
    if (_switch) {
        QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git switch -c " + dst + " " + src));
        terminalStdin(QStringList{"switch", "-c", dst, src});
    } else {
        QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git branch " + dst + " " + src));
        terminalStdin(QStringList{"branch", dst, src});
    }
}

void GitModule::gitRename(const QString &src, const QString &dst) {
    m_command = Rename;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git branch -m " + src + " " + dst));
    terminalStdin(QStringList{"branch", "-m", src, dst});
}

void GitModule::gitDelete(const QString &name) {
    m_command = Delete;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git branch -D " + name));
    terminalStdin(QStringList{"branch", "-D", name});
}

// public: file
void GitModule::gitAdd(const QUrl &documentUrl) {
    m_command = Add;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git add " + relativePath));
    terminalStdin(QStringList{"add", documentPath});
}

void GitModule::gitAddAll() {
    m_command = Add;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git add ."));
    terminalStdin(QStringList{"add", "."});
}

void GitModule::gitReset(const QUrl &documentUrl) {
    m_command = Reset;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git reset " + relativePath));
    terminalStdin(QStringList{"reset", documentPath});
}

void GitModule::gitResetAll() {
    m_command = Reset;
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git reset"));
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
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, QString("commit -m %1").arg("test")));
    terminalStdin(QStringList{"commit", "-m", "test"});
}

// private
void GitModule::terminalStdin(const QStringList &arguments) const {
    m_process->start("git", arguments);
}

void GitModule::terminalStdout() {
    const auto output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_root, "terminalStdout", Q_ARG(QVariant, output));
    const auto command = m_command;
    switch (command) {
        case Branch: {
            m_command = Null;
            m_standardItemModel->clear();
            auto *localItem = new QStandardItem(tr("Local")); // NOLINT
            localItem->setData("local", Qt::UserRole + 1);
            m_standardItemModel->appendRow(localItem);
            auto *remoteItem = new QStandardItem(tr("Remote")); // NOLINT
            remoteItem->setData("remote", Qt::UserRole + 1);
            m_standardItemModel->appendRow(remoteItem);
            for (const auto &value: output.split('\n')) {
                QString branch{};
                QString type = "untracked";
                if (value.startsWith('*')) {
                    type = "current";
                }
                branch = value.mid(2);
                const auto param = branch.split(' ', Qt::SkipEmptyParts);
                if (param.size() != 3) continue;
                const auto &name = param[0];
                const auto &hash = param[1];
                const auto &commit = param[2];
                auto *item = new QStandardItem(name); // NOLINT
                item->setData(type, Qt::UserRole + 1);
                item->setData(hash, Qt::UserRole + 2);
                item->setData(commit, Qt::UserRole + 3);
                if (name == "master") localItem->insertRow(0, item);
                else localItem->appendRow(item);
            }
            QMetaObject::invokeMethod(m_root, "branchExpand");
        }
        break;
        default: break;
    }
}

void GitModule::terminalStderr() const {
    const auto error = QString::fromLocal8Bit(m_process->readAllStandardError());
    QMetaObject::invokeMethod(m_root, "terminalStderr", Q_ARG(QVariant, error));
}

void GitModule::processFinished(const int exitcode) {
    QMetaObject::invokeMethod(m_root, "processFinished");
    const auto command = m_command;
    switch (command) {
        case Init: {
            m_command = Null;
            g_globalManager->gitSet();
            emit undateGit();
        }
        case Switch:
        case Create:
        case Rename:
        case Delete: {
            m_command = Null;
            gitBranch();
        }
        break;
        case Add:
        case Reset:
        case Commit: {
            m_command = Null;
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
