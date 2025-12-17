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
      m_explorerFileSystemModel(new QFileSystemModel()),
      m_explorerTreeView(new QTreeView()) {
    setWidget(m_explorerWidget);
}

void ExplorerModule::propertySet(const QVariantMap &objects) {
    m_scriptErrorDialog = qvariant_cast<QObject *>(objects["explorerModuleScriptErrorDialog"]);
    m_folderErrorDialog = qvariant_cast<QObject *>(objects["explorerModuleFolderErrorDialog"]);
    m_explorerWidget->rootContext()->setContextProperty("scriptMenu", qvariant_cast<QObject *>(objects["explorerModuleScriptMenu"]));
    m_explorerWidget->rootContext()->setContextProperty("folderMenu", qvariant_cast<QObject *>(objects["explorerModuleFolderMenu"]));
    m_explorerWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["explorerModuleRootMenu"]));

    const auto rootPath = g_workspaceUrl.toLocalFile();
    m_explorerFileSystemModel->setRootPath(rootPath);
    const QModelIndex fileRootIndex = m_explorerFileSystemModel->index(rootPath);
    m_explorerWidget->rootContext()->setContextProperty("explorerModule", this);
    m_explorerWidget->rootContext()->setContextProperty("fileRootIndex", fileRootIndex);
    m_explorerWidget->rootContext()->setContextProperty("fileSystemModel", m_explorerFileSystemModel);
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

void ExplorerModule::scriptNew(const QString &rootPath, const QString &scriptName) {
    QString filePath{};
    if (rootPath.isEmpty()) {
        filePath = QDir(m_explorerFileSystemModel->rootPath()).filePath(scriptName + ".lua");
    } else {
        filePath = QDir(rootPath).filePath(scriptName + ".lua");
    }
    if (QFile::exists(filePath)) {
        QMetaObject::invokeMethod(m_scriptErrorDialog, "open");
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        file.close();
        const auto fileUrl = QUrl::fromLocalFile(filePath);
        emit appendLog(QString("script created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
        const QUrl scriptUrl = QUrl::fromLocalFile(filePath).toString();
        emit openScript(scriptUrl);
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] script created at %2").arg(timestamp, fileUrl.toString());
    }
}

void ExplorerModule::scriptDelete(const QString &scriptPath) {
    QFile file(scriptPath);
    file.remove();
}

void ExplorerModule::folderNew(const QString &rootPath, const QString &folderName) {
    QString folderPath{};
    if (rootPath.isEmpty()) {
        folderPath = QDir(m_explorerFileSystemModel->rootPath()).filePath(folderName);
    } else {
        folderPath = QDir(rootPath).filePath(folderName);
    }
    if (QFile::exists(folderPath)) {
        QMetaObject::invokeMethod(m_folderErrorDialog, "open");
        return;
    }
    if (QDir().mkdir(folderPath)) {
        const auto fileUrl = QUrl::fromLocalFile(folderPath);
        emit appendLog(QString("folder created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] folder created at %2").arg(timestamp, fileUrl.toString());
    }
}

void ExplorerModule::folderDelete(const QString &folderPath) {
    QDir dir(folderPath);
    dir.removeRecursively();
}

void ExplorerModule::openInExplorer() const {
    const QDir folderPath = m_explorerFileSystemModel->rootPath();
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
