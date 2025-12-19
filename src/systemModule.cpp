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
}

void SystemModule::processTerminate() const {
    // const auto state = m_process->state();
    // qDebug() << state;
    m_process->terminate();
}

void SystemModule::resourceDelete(const QUrl &fileUrl) {
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

void SystemModule::resourceRename(const QUrl &fileUrl, const QString &name) {
    qDebug() << fileUrl << name;
}

void SystemModule::resourceOpenInExplorer(const QUrl &fileUrl) {
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

void SystemModule::resourceOpenInApplication(const QUrl &fileUrl) {
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

void SystemModule::copyToClipboard(const QUrl &fileUrl) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fileUrl.toString());
}
