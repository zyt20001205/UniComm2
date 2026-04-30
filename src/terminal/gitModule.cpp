#include "terminal/gitModule.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTextDocument>

#include "globals.h"

// public
GitModule::GitModule()
    : DockWidget("Git"),
      m_config(g_workspaceConfig["gitConfig"].toObject()),
      m_widget(new QQuickWidget()),
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

void GitModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("gitModule", this);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/gitModule.qml"));
    m_root = m_widget->rootObject();
    m_root->setProperty("gitEnabled", g_gitEnabled);
}

void GitModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
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

void GitModule::gitAdd(const QUrl &documentUrl) {
    m_command = Add;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git add " + relativePath));
    terminalStdin(QStringList{"add", documentPath});
}

void GitModule::gitReset(const QUrl &documentUrl) {
    m_command = Reset;
    const auto documentPath = documentUrl.toLocalFile();
    const auto workspaceDir = QDir(g_workspaceUrl.toLocalFile());
    const auto relativePath = workspaceDir.relativeFilePath(documentPath);
    QMetaObject::invokeMethod(m_root, "terminalStdin", Q_ARG(QVariant, "git reset " + relativePath));
    terminalStdin(QStringList{"reset", documentPath});
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
    for (const QString &line : gitignoreList) {
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
    parser(true);
    const auto output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    QMetaObject::invokeMethod(m_root, "terminalStdout", Q_ARG(QVariant, output));
}

void GitModule::terminalStderr() {
    parser(false);
    const auto error = QString::fromLocal8Bit(m_process->readAllStandardError());
    QMetaObject::invokeMethod(m_root, "terminalStderr", Q_ARG(QVariant, error));
}

void GitModule::processFinished(const int exitcode) {
    parser(exitcode == 0);
    QMetaObject::invokeMethod(m_root, "processFinished");
}

void GitModule::parser(const bool status) {
    const auto command = m_command;
    m_command = Null;
    switch (command) {
        case Init: {
            m_root->setProperty("gitEnabled", status);
            g_gitEnabled = status;
            emit initGit(status);
            emit undateGit();
        }
        break;
        case Add:
        case Reset:
        case Commit: {
            emit undateGit();
        }
        break;
        default: break;
    }
}
