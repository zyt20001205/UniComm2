#include "logModule.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPrinter>
#include <QProcess>
#include <QQmlContext>
#include <QQuickTextDocument>
#include <QQuickWidget>
#include <QStandardPaths>

#include "globals.h"

// LogModule public
LogModule::LogModule()
    : DockWidget("log"),
      m_logConfig(g_workspaceConfig["logConfig"].toObject()),
      m_logWidget(new QQuickWidget()),
      m_logTextDocument(new QTextDocument()) {
    setWidget(m_logWidget);
}

void LogModule::propertySet(const QVariantMap &objects) {
    m_heightDialog = qvariant_cast<QObject *>(objects["logModuleHeightDialog"]);
    m_logWidget->rootContext()->setContextProperty("logModule", this);
    m_logWidget->rootContext()->setContextProperty("heightDialog", m_heightDialog);
    m_logWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_logWidget->setSource(QUrl("qrc:/qml/logModule.qml"));
}

void LogModule::propertyGet(const QVariantMap &objects) {
    // set timestamp
    auto *timestampButton = qvariant_cast<QObject *>(objects["timestampButton"]);
    timestampButton->setProperty("checked", m_logConfig["timestamp"].toBool());
    // set font
    m_logTextArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto logFont = QFont(m_logConfig["fontFamily"].toString(), m_logConfig["fontSize"].toInt());
    m_logTextArea->setProperty("font", logFont);
    // set height
    const auto *quickTextDocument = qvariant_cast<QQuickTextDocument *>(m_logTextArea->property("textDocument"));
    m_logTextDocument = quickTextDocument->textDocument();
    m_logTextDocument->setMaximumBlockCount(m_logConfig["height"].toInt());
}

void LogModule::logConfigSave() const {
    g_workspaceConfig["logConfig"] = m_logConfig;
}

void LogModule::logFontReload(const QJsonObject &fontConfigLog) const {
    const auto logFont = QFont(fontConfigLog["fontFamily"].toString(), fontConfigLog["fontSize"].toInt());
    m_logTextArea->setProperty("font", logFont);
}

void LogModule::logFontSave(const QJsonObject &fontConfigLog) {
    m_logConfig["fontFamily"] = fontConfigLog["fontFamily"].toString();
    m_logConfig["fontSize"] = fontConfigLog["fontSize"].toInt();
}

void LogModule::logAppend(const QString &message, const QString &level) {
    // check timestamp
    QString timestamp = "";
    if (m_logConfig["timestamp"].toBool()) {
        timestamp = QString("[%1] ").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
    }
    // logging
    QString f_message = QString("%1%2").arg(timestamp, message);
    // check level
    if (level == "info")
        f_message = QString("<span style='color:black'>%1</span>").arg(f_message);
    else if (level == "warning")
        f_message = QString("<span style='color:orange'>%1</span>").arg(f_message);
    else if (level == "error")
        f_message = QString("<span style='color:red'>%1</span>").arg(f_message);
    else if (level == "tx")
        // f_message = QString("<span style='background-color:cyan;'>%1</span>").arg(f_message);
        f_message = QString("%1").arg(f_message);
    else //(level == "rx")
        // f_message = QString("<span style='background-color:lightgreen;'>%1</span>").arg(f_message);
        f_message = QString("%1").arg(f_message);
    // append log
    QMetaObject::invokeMethod(m_logTextArea, "append", Q_ARG(QString, f_message));
}

void LogModule::timestampToggle(const bool status) {
    m_logConfig["timestamp"] = status;
}

QString LogModule::heightRead() {
    return QString::number(m_logConfig["height"].toInt());
}

void LogModule::heightWrite(const QString &height) {
    m_logConfig["height"] = height.toInt();
    m_logTextDocument->setMaximumBlockCount(height.toInt());
}

void LogModule::logSave(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    QTextDocument document;
    document.setHtml(m_logTextArea->property("text").toString());
    if (document.toPlainText().isEmpty()) {
        QMessageBox::warning(nullptr, "Warning", tr("Log is empty."));
        return;
    }

    if (filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << document.toPlainText();
            file.close();
            logAppend(QString("log saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log saved to %2").arg(timestamp, filePath);
        } else {
            logAppend("log save failed", "info");
            // logging
            const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log save failed").arg(timestamp);
        }
    } else if (filePath.endsWith(".pdf", Qt::CaseInsensitive)) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        document.print(&printer);
        if (QFile::exists(filePath)) {
            logAppend(QString("log saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log saved to %2").arg(timestamp, filePath);
        } else {
            logAppend("log save failed", "info");
            // logging
            const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log save failed").arg(timestamp);
        }
    } else if (filePath.endsWith(".html", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << document.toHtml();
            file.close();
            logAppend(QString("log saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log saved to %2").arg(timestamp, filePath);
        } else {
            logAppend("log save failed", "info");
            // logging
            const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log save failed").arg(timestamp);
        }
    }
}

void LogModule::urlCopy(const QUrl &fileUrl) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(fileUrl.toString());
}

void LogModule::openInExplorer(const QUrl &fileUrl) {
    const QString folderPath = QFileInfo(fileUrl.toLocalFile()).absolutePath();
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    QStringList args;
    args << QDir::toNativeSeparators(folderPath);
    QProcess::startDetached(command, args);
#endif
}

void LogModule::openInApplication(const QUrl &fileUrl) {
#ifdef Q_OS_WIN
    const QString command = "explorer.exe";
    QStringList args;
    args << QDir::toNativeSeparators(fileUrl.toLocalFile());
    QProcess::startDetached(command, args);
#endif
}