#include "logModule.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPushButton>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardPaths>
#include <QTextDocumentWriter>
#include <QTextBrowser>

#include "globals.h"
// LogModule public
LogModule::LogModule()
    : DockWidget("log"),
      m_logConfig(g_workspaceConfig["logConfig"].toObject()),
      m_logWidget(new QQuickWidget()),
      m_logTextBrowser(new QTextBrowser()) {
    setWidget(m_logWidget);
    m_logWidget->rootContext()->setContextProperty("logModule", this);
    m_logWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_logWidget->setSource(QUrl("qrc:/qml/logModule.qml"));
    const QQuickItem *rootObject = m_logWidget->rootObject();
    m_logTextArea = rootObject->findChild<QObject *>("logTextArea");
    const auto logFont = QFont(m_logConfig["fontFamily"].toString(), m_logConfig["fontSize"].toInt());
    m_logTextArea->setProperty("font", logFont);
    auto *timestampButton = rootObject->findChild<QObject *>("timestampButton");
    timestampButton->setProperty("checked", m_logConfig["timestamp"].toBool());

    // auto *widget = new QWidget(); // NOLINT
    // auto *layout = new QHBoxLayout(widget); // NOLINT
    // setWidget(widget);

    // auto *ctrlWidget = new QWidget(); // NOLINT
    // layout->addWidget(ctrlWidget);
    // auto *ctrlLayout = new QVBoxLayout(ctrlWidget); // NOLINT
    // ctrlLayout->setContentsMargins(0, 0, 0, 0);
    // ctrlLayout->setAlignment(Qt::AlignTop);
    // auto *timestampButton = new QPushButton(); // NOLINT
    // ctrlLayout->addWidget(timestampButton);
    // timestampButton->setFixedSize(24, 24);
    // timestampButton->setIcon(QIcon(":/icon/clock.svg"));
    // timestampButton->setToolTip(tr("timestamp"));
    // timestampButton->setCheckable(true);
    // timestampButton->setChecked(m_logConfig["timestamp"].toBool());
    // connect(timestampButton, &QPushButton::clicked, this, [this,timestampButton] {
    //     m_logConfig["timestamp"] = timestampButton->isChecked();
    // });
    // auto *heightButton = new QPushButton(); // NOLINT
    // ctrlLayout->addWidget(heightButton);
    // heightButton->setFixedSize(24, 24);
    // heightButton->setIcon(QIcon(":/icon/autoFitHeight.svg"));
    // heightButton->setToolTip(tr("maximum line count"));
    // connect(heightButton, &QPushButton::clicked, this, [this] {
    //     bool ok = false;
    //     const int height = QInputDialog::getInt(nullptr, "Log Setting", "maximum line count:", m_logConfig["height"].toInt(), 1, 10000, 1, &ok);
    //     if (ok) {
    //         m_logTextBrowser->document()->setMaximumBlockCount(height);
    //         m_logConfig["height"] = height;
    //     }
    // });
    // auto *saveButton = new QPushButton(); // NOLINT
    // ctrlLayout->addWidget(saveButton);
    // saveButton->setFixedSize(24, 24);
    // saveButton->setIcon(QIcon(":/icon/save.svg"));
    // saveButton->setToolTip(tr("save log"));
    // connect(saveButton, &QPushButton::clicked, this, &LogModule::logSave);
    // auto *clearButton = new QPushButton(); // NOLINT
    // ctrlLayout->addWidget(clearButton);
    // clearButton->setFixedSize(24, 24);
    // clearButton->setIcon(QIcon(":/icon/delete.svg"));
    // clearButton->setToolTip(tr("clear log"));
    // connect(clearButton, &QPushButton::clicked, this, &LogModule::logClear);
    //
    // layout->addWidget(m_logTextBrowser);
    // const auto logFont = QFont(m_logConfig["fontFamily"].toString(), m_logConfig["fontSize"].toInt());
    // m_logTextBrowser->setFont(logFont);
    // m_logTextBrowser->setOpenExternalLinks(false);
    // m_logTextBrowser->setOpenLinks(false);
    // m_logTextBrowser->document()->setMaximumBlockCount(m_logConfig["height"].toInt());
    // connect(m_logTextBrowser, &QTextBrowser::anchorClicked, this, [](const QUrl &link) { QDesktopServices::openUrl(link); });
}

void LogModule::logConfigSave() const {
    g_workspaceConfig["logConfig"] = m_logConfig;
}

void LogModule::logFontReload(const QJsonObject &fontConfigLog) const {
    const auto logFont = QFont(fontConfigLog["fontFamily"].toString(), fontConfigLog["fontSize"].toInt());
    m_logTextArea->setProperty("font", logFont);
    m_logTextBrowser->setFont(logFont);
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
    // }
    // check level
    if (level == "info")
        f_message = QString("<span style='color:black'>%1</span>").arg(f_message);
    else if (level == "warning")
        f_message = QString("<span style='color:orange'>%1</span>").arg(f_message);
    else if (level == "error")
        f_message = QString("<span style='color:red'>%1</span>").arg(f_message);
    else if (level == "tx")
        f_message = QString("<span style='background-color:cyan;'>%1</span>").arg(f_message);
    else //(level == "rx")
        f_message = QString("<span style='background-color:lightgreen;'>%1</span>").arg(f_message);
    // append log
    QMetaObject::invokeMethod(m_logTextArea, "append", Q_ARG(QString, f_message));
    m_logTextBrowser->append(f_message);
}

void LogModule::timestampToggle(const bool status) {
    m_logConfig["timestamp"] = status;
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
            logAppend(QString("log saved to %1").arg(filePath), "info");
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
            logAppend(QString("log saved to %1").arg(filePath), "info");
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
            logAppend(QString("log saved to %1").arg(filePath), "info");
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
