#include "systemModule.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

// SystemModule public
SystemModule::SystemModule(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this)) {
}

void SystemModule::propertySet(const QVariantMap &objects) {
    m_busyDialog = qvariant_cast<QObject *>(objects["mainWindowBusyDialog"]);
    m_errorDialog = qvariant_cast<QObject *>(objects["systemModuleErrorDialog"]);
}

void SystemModule::processTerminate() const {
    // const auto state = m_process->state();
    // qDebug() << state;
    m_process->terminate();
}

void SystemModule::fileOpenInExplorer(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    QStringList args;
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    if (fileInfo.isFile()) {
        args << "/select," << QDir::toNativeSeparators(filePath);
    } else if (fileInfo.isDir()) {
        args << QDir::toNativeSeparators(filePath);
    }
#endif
    connect(m_process, &QProcess::started, this, [this] {
        m_busyDialog->setProperty("title", tr("Waiting for explorer..."));
        QMetaObject::invokeMethod(m_busyDialog, "open");
    });
    connect(m_process, &QProcess::finished, this, [this] {
        m_busyDialog->setProperty("title", "");
        QMetaObject::invokeMethod(m_busyDialog, "close");
    });
    m_process->start(command, args);
}

void SystemModule::fileOpenInApplication(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    QStringList args;
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    args << QDir::toNativeSeparators(filePath);
#endif
    connect(m_process, &QProcess::started, this, [this] {
        m_busyDialog->setProperty("title", tr("Waiting for application..."));
        QMetaObject::invokeMethod(m_busyDialog, "open");
    });
    connect(m_process, &QProcess::finished, this, [this] {
        m_busyDialog->setProperty("title", "");
        QMetaObject::invokeMethod(m_busyDialog, "close");
    });
    m_process->start(command, args);
}

void SystemModule::fileNew(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        if (fileInfo.exists()) {
            m_errorDialog->setProperty("title", tr("File already exists"));
            QMetaObject::invokeMethod(m_errorDialog, "open");
        } else {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
                emit openScript(fileUrl);
                emit appendLog(QString("file created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] file created at %2").arg(timestamp, fileUrl.toString());
            }
        }
    } else {
        if (fileInfo.exists()) {
            m_errorDialog->setProperty("title", tr("Dir already exists"));
            QMetaObject::invokeMethod(m_errorDialog, "open");
        } else {
            const QDir dir;
            if (dir.mkpath(filePath)) {
                emit appendLog(QString("folder created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] folder created at %2").arg(timestamp, fileUrl.toString());
            }
        }
    }
}

void SystemModule::fileRename(const QUrl &fileUrl, const QString &name) const {
    qDebug() << fileUrl << name;
}

void SystemModule::fileDelete(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    if (fileInfo.isFile()) {
        QFile file(filePath);
        file.remove();
    } else if (fileInfo.isDir()) {
        QDir dir(filePath);
        dir.removeRecursively();
    }
}

void SystemModule::copyToClipboard(const QUrl &fileUrl) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fileUrl.toString());
}
