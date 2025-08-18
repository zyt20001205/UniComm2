#include "../include/explorer.h"

Explorer::Explorer(QObject *parent)
    : QDockWidget("explorer", qobject_cast<QWidget *>(parent)) {
    m_treeView = new QTreeView();
    setWidget(m_treeView);
    m_treeView->installEventFilter(this);
    connect(m_treeView, &QTreeView::doubleClicked, this, &Explorer::fileDoubleClicked);

    m_model = new QFileSystemModel();
    m_treeView->setModel(m_model);
    m_treeView->setColumnHidden(2, true);
    m_treeView->setColumnHidden(3, true);
    m_model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    const QString scriptPath = QDir::currentPath() + "/script";

    // check if script dir exists
    if (const QDir scriptDir(scriptPath); !scriptDir.exists()) {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "script directory generated");
        if (!scriptDir.mkpath(".")) {
            // logging
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] %2").arg(timestamp, "script directory generation failed");
            return;
        }
    } else {
        // logging
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2").arg(timestamp, "script directory found");
    }

    m_model->setRootPath(scriptPath);
    m_treeView->setRootIndex(m_model->index(scriptPath));
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "script directory loaded");
}

void Explorer::fileDoubleClicked(const QModelIndex &index) {
    const QString filePath = m_model->filePath(index);
    emit loadScript(filePath);
}

void Explorer::fileDelete(const QModelIndex &index) {
    const QString filePath = m_model->filePath(index);

    if (QFile::exists(filePath)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, tr("Delete Script"), tr("Are you sure to delete script?"), QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    QFile::remove(filePath);

    const QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    emit appendLog(QString("%1 %2").arg(fileName, "deleted"), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 %3").arg(timestamp, fileName, "deleted");
}

bool Explorer::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_treeView && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            if (const QModelIndex index = m_treeView->currentIndex(); index.isValid()) {
                fileDelete(index);
                return true;
            }
        }
    }
    return QDockWidget::eventFilter(obj, event);
}
