#include "explorer.h"

#include "globals.h"

// Explorer public
Explorer::Explorer(QWidget *parent)
    : QDockWidget("explorer", parent),
      m_explorerTreeView(new QTreeView()),
      m_model(new QFileSystemModel()) {
    setWidget(m_explorerTreeView);

    m_explorerTreeView->installEventFilter(this);
    connect(m_explorerTreeView, &QTreeView::doubleClicked, this, &Explorer::scriptOpen);

    m_explorerTreeView->setModel(m_model);
    m_explorerTreeView->setHeaderHidden(true);
    m_explorerTreeView->setColumnHidden(1, true);
    m_explorerTreeView->setColumnHidden(2, true);
    m_explorerTreeView->setColumnHidden(3, true);
    m_explorerTreeView->setColumnHidden(4, true);
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
    // open workspace
    if (const QUrl rootUrl(g_config["mainConfig"].toObject()["workspace"].toString()); !rootUrl.isEmpty()) {
        workspaceOpen(rootUrl);
    }
}

void Explorer::workspaceOpen(const QUrl &rootUrl) const {
    const QString rootPath = rootUrl.toLocalFile();
    m_model->setRootPath(rootPath);
    m_explorerTreeView->QTreeView::setRootIndex(m_model->index(rootPath));
}

// Explorer protected
void Explorer::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint vpPos = m_explorerTreeView->viewport()->mapFromGlobal(event->globalPos());
    const QModelIndex index = m_explorerTreeView->indexAt(vpPos);
    QMenu menu(this);
    if (!index.isValid()) {
        menu.addAction(tr("new script"), this, &Explorer::scriptNew);
        menu.addAction(tr("open in explorer"), this, &Explorer::scriptOpenInExplorer);
    } else {
        menu.addAction(tr("run"), [this, index] { scriptRun(index); });
        menu.addAction(tr("debug"), [this, index] { scriptDebug(index); });
        menu.addAction(tr("open"), [this, index] { scriptOpen(index); });
        menu.addAction(tr("delete"), [this, index] { scriptDelete(index); });
    }
    menu.exec(event->globalPos());
}

bool Explorer::eventFilter(QObject *obj, QEvent *event) {
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

// Explorer private
void Explorer::scriptRun(const QModelIndex &index) {
    const QString scriptPath = m_model->filePath(index);
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    emit runScript(scriptUrl, script);
}

void Explorer::scriptDebug(const QModelIndex &index) {
    const QString scriptPath = m_model->filePath(index);
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    emit debugScript(scriptUrl, script);
}

void Explorer::scriptOpen(const QModelIndex &index) {
    const QString scriptPath = m_model->filePath(index);
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath).toString();
    emit openScript(scriptUrl);
}

void Explorer::scriptDelete(const QModelIndex &index) {
    const QString fileName = m_model->fileName(index);
    const QMessageBox::StandardButton reply =
            QMessageBox::question(nullptr, tr("Delete Script"), tr("Are you sure to delete script?"), QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    if (!m_model->remove(index)) {
        emit appendLog(QString("%1 %2").arg(fileName, "delete failed"), "info");
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "delete failed");
        return;
    }
    emit appendLog(QString("%1 %2").arg(fileName, "deleted"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "deleted");
}

void Explorer::scriptNew() {
    bool ok;
    QString fileName = QInputDialog::getText(nullptr, "New Script", "script name:", QLineEdit::Normal, "new", &ok);
    if (!ok || fileName.isEmpty()) {
        return;
    }
    fileName += ".lua";
    const QString filePath = QDir::current().filePath(m_model->rootPath() + "/" + fileName);

    if (QFile::exists(filePath)) {
        const QMessageBox::StandardButton reply =
                QMessageBox::question(nullptr, tr("File Exists"), tr("File already exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    file.close();

    emit appendLog(QString("%1 %2").arg(fileName, "created"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "created");
}

void Explorer::scriptOpenInExplorer() const {
    const QDir folderPath = m_model->rootPath();
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