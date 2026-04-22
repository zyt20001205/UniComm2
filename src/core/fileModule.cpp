#include "core/fileModule.h"

#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QProcess>
#include <QUrl>

#include "globals.h"

// public
FileModule::FileModule(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this)) {
}

void FileModule::propertySet(const QVariantMap &objects) {
    m_busyDialog = qvariant_cast<QObject *>(objects["mainWindowBusyDialog"]);
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
}

void FileModule::processTerminate() const {
    // const auto state = m_process->state();
    // qDebug() << state;
    m_process->terminate();
}

void FileModule::fileOpenInExplorer(const QUrl &fileUrl) {
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

void FileModule::fileOpenInApplication(const QUrl &fileUrl) {
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

QVariantHash FileModule::fileInfo(const QUrl &fileUrl) {
    const auto filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    QUrl source{};
    const auto suffix = fileInfo.suffix();
    const QStringList imageType = {"bmp", "gif", "ico", "jpeg", "jpg", "png", "svg", "tif", "tiff", "webp"};
    if (imageType.contains(suffix)) {
        source = "qrc:/icon/fileTypeImage.svg";
    } else if (suffix == "csv") {
        source = "qrc:/icon/fileTypeCsv.svg";
    } else if (suffix == "json") {
        source = "qrc:/icon/fileTypeJson.svg";
    } else if (suffix == "lua") {
        source = "qrc:/icon/fileTypeLua.svg";
    } else if (fileInfo.isFile()) {
        source = "qrc:/icon/document.svg";
    } else if (fileInfo.isDir()) {
        source = "qrc:/icon/folder.svg";
    }
    const QLocale locale{QLocale::C};
    QVariantHash infoSession = {
        {"source", source},
        {"baseName", fileInfo.baseName()},
        {"absolutePath", fileInfo.absoluteFilePath()},
        {"size", locale.formattedDataSize(fileInfo.size())},
        {"birthTime", fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss")},
        {"lastModified", fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")},
        {"lastRead", fileInfo.lastRead().toString("yyyy-MM-dd HH:mm:ss")},
        {"readable", fileInfo.isReadable()},
        {"writable", fileInfo.isWritable()},
        {"hidden", fileInfo.isHidden()},
    };
    return infoSession;
}

void FileModule::fileWritable(const QUrl &fileUrl, const bool status) {
    const auto filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    auto permissions = fileInfo.permissions();
    if (status) permissions|= QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther;
    else permissions &= ~(QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther);
    QFile::setPermissions(filePath, permissions);
    emit setPermission(fileUrl);
}

void FileModule::fileNew(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.suffix().isEmpty()) {
        if (fileInfo.exists()) {
            m_messageDialog->setProperty("title", tr("Error"));
            m_messageDialog->setProperty("text", tr("File already exists."));
            QMetaObject::invokeMethod(m_messageDialog, "open");
        } else {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
                emit openDocument(fileUrl);
                emit appendLog(LOG_INFO, QString("file created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "");
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] file created at %2").arg(timestamp, fileUrl.toString());
            }
        }
    } else {
        if (fileInfo.exists()) {
            m_messageDialog->setProperty("title", tr("Error"));
            m_messageDialog->setProperty("text", tr("Dir already exists."));
            QMetaObject::invokeMethod(m_messageDialog, "open");
        } else {
            const QDir dir;
            if (dir.mkpath(filePath)) {
                emit appendLog(LOG_INFO, QString("folder created at <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "");
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] folder created at %2").arg(timestamp, fileUrl.toString());
            }
        }
    }
}

void FileModule::fileRename(const QUrl &fileUrl, const QString &name) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    if (fileInfo.isFile()) {
        QFile file(filePath);
        const QString newPath = fileInfo.dir().filePath(name + "." + fileInfo.suffix());
        if (file.rename(newPath)) {
            const auto newUrl = QUrl::fromLocalFile(newPath);
            didRenameFilesNotification(fileUrl, newUrl);
            emit appendLog(LOG_INFO, QString("file renamed to <a href='%1'>%2</a>").arg(newUrl.toString(), newUrl.toString()), "");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] file renamed to %2").arg(timestamp, newUrl.toString());
        }
    } else if (fileInfo.isDir()) {
        QFile file(filePath);
        const QString newPath = fileInfo.dir().filePath(name);
        if (file.rename(newPath)) {
            const auto newUrl = QUrl::fromLocalFile(newPath);
            didRenameFilesNotification(fileUrl, newUrl);
            emit appendLog(LOG_INFO, QString("folder renamed to <a href='%1'>%2</a>").arg(newUrl.toString(), newUrl.toString()), "");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] folder renamed to %2").arg(timestamp, newUrl.toString());
        }
    }
}

void FileModule::fileDelete(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo fileInfo(filePath);
    QString trashPath{};
    if (fileInfo.isFile()) {
        if (QFile::moveToTrash(filePath, &trashPath)) {
            didDeleteFilesNotification(fileUrl);
            const auto trashUrl = QUrl::fromLocalFile(trashPath);
            emit appendLog(LOG_INFO, QString("file deleted <a href='%1'>%2</a>").arg(trashUrl.toString(), trashUrl.toString()), "");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] file deleted %2").arg(timestamp, trashUrl.toString());
        }
    } else if (fileInfo.isDir()) {
        if (QFile::moveToTrash(filePath, &trashPath)) {
            didDeleteFilesNotification(fileUrl);
            const auto trashUrl = QUrl::fromLocalFile(trashPath);
            emit appendLog(LOG_INFO, QString("folder deleted <a href='%1'>%2</a>").arg(trashUrl.toString(), trashUrl.toString()), "");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] folder deleted %2").arg(timestamp, trashUrl.toString());
        }
    }
}

void FileModule::copyToClipboard(const QUrl &fileUrl) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(fileUrl.toString());
}

QString FileModule::textGet(const QUrl &documentUrl, const int startLine, const int startCharacter, const int endLine, const int endCharacter) {
    // TODO: character not implemented
    QFile file(documentUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    QTextStream in(&file);
    if (startLine == -1) return in.readAll();
    QStringList lines{};
    while (!in.atEnd()) {
        const auto line = in.readLine();
        lines.append(line);
    }
    QString text{};
    QTextStream out(&text);
    for (int i = startLine; i <= endLine; ++i) {
        out << lines.at(i);
        if (i < endLine) out << Qt::endl;
    }
    return text;
}

void FileModule::didRenameFilesNotification(const QUrl &oldUrl, const QUrl &newUrl) {
    // did rename files notification to lua language server
    const QJsonObject didRenameFilesParams{
        {
            "files", QJsonArray{
                QJsonObject{
                    {"oldUri", oldUrl.toString()},
                    {"newUri", newUrl.toString()}
                }
            }
        }
    };
    emit notificationJson("workspace/didRenameFiles", didRenameFilesParams);
}

void FileModule::didDeleteFilesNotification(const QUrl &fileUrl) {
    // did delete files notification to lua language server
    const QJsonObject didDeleteFilesParams{
        {
            "files", QJsonArray{
                QJsonObject{
                    {"uri", fileUrl.toString()}
                }
            }
        }
    };
    emit notificationJson("workspace/didDeleteFiles", didDeleteFilesParams);
}
