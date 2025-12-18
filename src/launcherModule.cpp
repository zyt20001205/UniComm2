#include "launcherModule.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

// LauncherModule public
LauncherModule::LauncherModule(QObject *parent)
    : QObject(parent) {
}

void LauncherModule::openInExplorer(const QUrl &fileUrl) {
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
    QProcess::startDetached(command, args);
#endif
}

void LauncherModule::openInApplication(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    QStringList args;
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    args << QDir::toNativeSeparators(filePath);
    QProcess::startDetached(command, args);
#endif
}

void LauncherModule::copyToClipboard(const QUrl &fileUrl) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fileUrl.toString());
}
