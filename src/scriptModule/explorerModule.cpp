#include "scriptModule/explorerModule.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QTreeView>

#include "globals.h"

// ExplorerModule public
ExplorerModule::ExplorerModule()
    : DockWidget("explorer"),
      m_explorerTreeView(new QTreeView()),
      m_explorerTreeModel(new QFileSystemModel()) {
    setWidget(m_explorerTreeView);

    m_explorerTreeView->installEventFilter(this);
    connect(m_explorerTreeView, &QTreeView::doubleClicked, this, &ExplorerModule::scriptOpen);

    m_explorerTreeView->setModel(m_explorerTreeModel);
    m_explorerTreeView->setHeaderHidden(true);
    m_explorerTreeView->setColumnHidden(1, true);
    m_explorerTreeView->setColumnHidden(2, true);
    m_explorerTreeView->setColumnHidden(3, true);
    m_explorerTreeView->setColumnHidden(4, true);
    m_explorerTreeModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
}

void ExplorerModule::workspaceOpen(const QUrl &rootUrl) const {
    const QString rootPath = rootUrl.toLocalFile();
    m_explorerTreeModel->setRootPath(rootPath);
    m_explorerTreeView->QTreeView::setRootIndex(m_explorerTreeModel->index(rootPath));
}

// ExplorerModule protected
void ExplorerModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint vpPos = m_explorerTreeView->viewport()->mapFromGlobal(event->globalPos());
    const QModelIndex index = m_explorerTreeView->indexAt(vpPos);
    QMenu menu(this);
    if (!index.isValid()) {
        menu.addAction(tr("New Script"), this, [this] { scriptNew(); });
        menu.addAction(tr("New Folder"), this, [this] { folderNew(); });
        menu.addAction(tr("Open In Explorer"), this, &ExplorerModule::scriptOpenInExplorer);
    } else {
        if (const QFileInfo fileInfo = m_explorerTreeModel->fileInfo(index); fileInfo.isDir()) {
            menu.addAction(tr("New Script"), this, [this, fileInfo] {
                const QString rootPath = fileInfo.absoluteFilePath();
                scriptNew(rootPath);
            });
            menu.addAction(tr("New Folder"), this, [this, fileInfo] {
                const QString rootPath = fileInfo.absoluteFilePath();
                folderNew(rootPath);
            });
            menu.addAction(tr("Delete Folder"), [this, index] { folderDelete(index); });
        } else {
            if (const QString fileSuffix = fileInfo.suffix(); fileSuffix == "lua") {
                menu.addAction(tr("Run Script"), [this, index] { scriptRun(index); });
                menu.addAction(tr("Debug Script"), [this, index] { scriptDebug(index); });
                menu.addAction(tr("Open Script"), [this, index] { scriptOpen(index); });
                menu.addAction(tr("Delete Script"), [this, index] { scriptDelete(index); });
            }
        }
    }
    menu.exec(event->globalPos());
}

bool ExplorerModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == this && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            if (const QModelIndex index = m_explorerTreeView->currentIndex(); index.isValid()) {
                scriptDelete(index);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

// ExplorerModule private
void ExplorerModule::scriptRun(const QModelIndex &index) {
    const QString scriptPath = m_explorerTreeModel->filePath(index);
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    emit runScript(scriptUrl, script);
}

void ExplorerModule::scriptDebug(const QModelIndex &index) {
    const QString scriptPath = m_explorerTreeModel->filePath(index);
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    emit debugScript(scriptUrl, script);
}

void ExplorerModule::scriptOpen(const QModelIndex &index) {
    const QString scriptPath = m_explorerTreeModel->filePath(index);
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
        rootPath = m_explorerTreeModel->rootPath();
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

void ExplorerModule::scriptDelete(const QModelIndex &index) {
    const QString fileName = m_explorerTreeModel->fileName(index);
    const QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr,
        tr("Delete Script"),
        tr("Are you sure to delete script %1?").arg(fileName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    if (!m_explorerTreeModel->remove(index)) {
        emit appendLog(QString("%1 delete failed").arg(fileName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 delete failed").arg(timestamp, fileName);
        return;
    }
    emit appendLog(QString("%1 deleted").arg(fileName), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 deleted").arg(timestamp, fileName);
}

void ExplorerModule::folderNew(QString rootPath) {
    bool ok;
    const QString folderName = QInputDialog::getText(nullptr, tr("New Folder"), tr("folder name:"), QLineEdit::Normal, "new folder", &ok);
    if (!ok || folderName.isEmpty()) {
        return;
    }

    if (rootPath.isEmpty()) {
        rootPath = m_explorerTreeModel->rootPath();
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

void ExplorerModule::folderDelete(const QModelIndex &index) {
    const QString folderName = m_explorerTreeModel->fileName(index);
    const QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr, tr("Delete Folder"),
        tr("Are you sure to delete folder %1 and all its contents?").arg(folderName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    if (!m_explorerTreeModel->remove(index)) {
        emit appendLog(QString("%1 delete failed").arg(folderName), "error");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 delete failed").arg(timestamp, folderName);
        return;
    }
    emit appendLog(QString("%1 deleted").arg(folderName), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 deleted").arg(timestamp, folderName);
}

void ExplorerModule::scriptOpenInExplorer() const {
    const QDir folderPath = m_explorerTreeModel->rootPath();
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
