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

void SystemModule::resourceDelete(const QUrl &resourceUrl) {
    const QString resourcePath = resourceUrl.toLocalFile();
    const QFileInfo resourceInfo(resourcePath);
    if (resourceInfo.isFile()) {
        QFile file(resourcePath);
        file.remove();
    } else if (resourceInfo.isDir()) {
        QDir dir(resourcePath);
        dir.removeRecursively();
    }
}

void SystemModule::resourceRename(const QUrl &resourceUrl, const QString &name) const {
    // const QString oldResourcePath = resourceUrl.toLocalFile();
    // const QFileInfo oldResourceInfo(oldResourcePath);
    // const QString newResourcePath = QDir(oldResourceInfo.absolutePath()).filePath(name);
    // const QFileInfo newResourceInfo(newResourcePath);
    // if (oldResourceInfo.isFile()) {
    //     if (newResourceInfo.exists()) {
    //         m_errorDialog->setProperty("title", tr("File already exists"));
    //         QMetaObject::invokeMethod(m_errorDialog, "open");
    //     } else {
    //         QFile file(oldResourcePath);
    //         file.rename(name);
    //     }
    // } else if (oldResourceInfo.isDir()) {
    //     if (newResourceInfo.exists()) {
    //         m_errorDialog->setProperty("title", tr("Dir already exists"));
    //         QMetaObject::invokeMethod(m_errorDialog, "open");
    //     } else {
    //         QDir dir;
    //         dir.rename(oldResourcePath, newResourcePath);
    //     }
    // }
}

void SystemModule::resourceOpenInExplorer(const QUrl &resourceUrl) {
    const QString resourcePath = resourceUrl.toLocalFile();
    const QFileInfo resourceInfo(resourcePath);
    QStringList args;
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    if (resourceInfo.isFile()) {
        args << "/select," << QDir::toNativeSeparators(resourcePath);
    } else if (resourceInfo.isDir()) {
        args << QDir::toNativeSeparators(resourcePath);
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

void SystemModule::resourceOpenInApplication(const QUrl &resourceUrl) {
    const QString resourcePath = resourceUrl.toLocalFile();
    QStringList args;
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    args << QDir::toNativeSeparators(resourcePath);
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

void SystemModule::copyToClipboard(const QUrl &resourceUrl) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(resourceUrl.toString());
}
