#include "scriptModule/codeEditor/explorerModule.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QQmlContext>
#include <QQuickWidget>
#include <QTreeView>

#include "globals.h"

// ExplorerModule public
ExplorerModule::ExplorerModule()
    : DockWidget("explorer"),
      m_explorerWidget(new QQuickWidget()),
      m_explorerFileModel(new QFileSystemModel()),
      m_explorerTreeView(new QTreeView()) {
    setWidget(m_explorerWidget);
    const auto rootPath = g_workspaceUrl.toLocalFile();
    m_explorerFileModel->setRootPath(rootPath);
    const QModelIndex fileRootIndex = m_explorerFileModel->index(rootPath);
    m_explorerWidget->rootContext()->setContextProperty("explorerModule", this);
    m_explorerWidget->rootContext()->setContextProperty("fileRootIndex", fileRootIndex);
    m_explorerWidget->rootContext()->setContextProperty("fileModel", m_explorerFileModel);
    m_explorerWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_explorerWidget->setSource(QUrl("qrc:/qml/scriptModule/codeEditor/explorerModule.qml"));
}

void ExplorerModule::scriptRun(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    QString threadId{};
    emit startThread(scriptUrl, LUATHREAD_RUN, threadId);
}

void ExplorerModule::scriptDebug(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    QString threadId{};
    emit startThread(scriptUrl, LUATHREAD_DEBUG, threadId);
}

void ExplorerModule::scriptOpen(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath).toString();
    emit openScript(scriptUrl);
}

void ExplorerModule::scriptNew(QString rootPath) {
    bool ok;
    QString fileName = QInputDialog::getText(nullptr, tr("New Script"), tr("script name:"), QLineEdit::Normal, "new script", &ok);
    if (!ok || fileName.isEmpty()) {
        return;
    }
    fileName += ".lua";

    if (rootPath.isEmpty()) {
        rootPath = m_explorerFileModel->rootPath();
    }
    const QString filePath = QDir(rootPath).filePath(fileName);

    if (QFile::exists(filePath)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr,
            tr("File Exists"),
            tr("File already exists. Overwrite?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    file.close();

    emit appendLog(QString("%1 created").arg(fileName), "info");
    const QUrl scriptUrl = QUrl::fromLocalFile(filePath).toString();
    emit openScript(scriptUrl);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 created").arg(timestamp, fileName);
}

void ExplorerModule::scriptDelete(const QString &scriptPath) {
    QFile file(scriptPath);
    file.remove();
}

void ExplorerModule::folderNew(QString rootPath) {
    bool ok;
    const QString folderName = QInputDialog::getText(nullptr, tr("New Folder"), tr("folder name:"), QLineEdit::Normal, "new folder", &ok);
    if (!ok || folderName.isEmpty()) {
        return;
    }

    if (rootPath.isEmpty()) {
        rootPath = m_explorerFileModel->rootPath();
    }
    const QString folderPath = QDir(rootPath).filePath(folderName);

    if (QFile::exists(folderPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Folder already exists."));
        return;
    }

    if (QDir().mkdir(folderPath)) {
        emit appendLog(QString("%1 created").arg(folderName), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 created").arg(timestamp, folderName);
    }
}

void ExplorerModule::folderDelete(const QString &folderPath) {
    QDir dir(folderPath);
    dir.removeRecursively();
}

void ExplorerModule::openInExplorer() const {
    const QDir folderPath = m_explorerFileModel->rootPath();
    const QString folderAbsolutePath = folderPath.absolutePath();
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    QStringList args;
    args << QDir::toNativeSeparators(folderAbsolutePath);
    QProcess::startDetached(command, args);
#endif
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "opened in explorer");
}
