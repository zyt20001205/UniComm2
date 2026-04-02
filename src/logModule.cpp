#include "logModule.h"

#include <QFileDialog>
#include <QPrinter>
#include <QQmlContext>
#include <QQuickTextDocument>
#include <QQuickWidget>
#include <QStandardPaths>

#include "globals.h"

// public
LogModule::LogModule()
    : DockWidget("Log"),
      m_logConfig(g_workspaceConfig["logConfig"].toObject()),
      m_logWidget(new QQuickWidget()),
      m_logTextDocument(new QTextDocument()) {
    setWidget(m_logWidget);
}

LogModule::~LogModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] log module destructed").arg(timestamp);
}

void LogModule::propertySet(const QVariantMap &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_logWidget->rootContext()->setContextProperty("heightDialog", qvariant_cast<QObject *>(objects["logModuleHeightDialog"]));
    m_logWidget->rootContext()->setContextProperty("linkMenu", qvariant_cast<QObject *>(objects["logModuleLinkMenu"]));
    m_logWidget->rootContext()->setContextProperty("mainTooltip", qvariant_cast<QObject *>(objects["mainWindowTooltip"]));

    m_logWidget->rootContext()->setContextProperty("logModule", this);
    m_logWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_logWidget->setSource(QUrl("qrc:/qml/logModule.qml"));
}

void LogModule::propertyGet(const QVariantMap &objects) {
    // set timestamp
    auto *timestampButton = qvariant_cast<QObject *>(objects["timestampButton"]);
    timestampButton->setProperty("checked", m_logConfig["timestamp"].toBool());
    // set wrap
    auto *wrapButton = qvariant_cast<QObject *>(objects["wrapButton"]);
    wrapButton->setProperty("checked", m_logConfig["wrap"].toBool());
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

void LogModule::logAppend(const QString &message, const int type) {
    // check timestamp
    QString timestamp = "";
    if (m_logConfig["timestamp"].toBool()) {
        timestamp = QString("[%1] ").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
    }
    // logging
    QString f_message = QString("%1%2").arg(timestamp, message);
    // check level
    switch (type) {
        case LOG_ERROR: {
            f_message = QString("<span style='color:red'>%1</span>").arg(f_message);
        }
        break;
        case LOG_WARNING: {
            f_message = QString("<span style='color:orange'>%1</span>").arg(f_message);
        }
        break;
        case LOG_INFO: {
            f_message = QString("<span style='color:black'>%1</span>").arg(f_message);
        }
        break;
        case LOG_TX: {
            f_message = QString("%1").arg(f_message);
        }
        break;
        case LOG_RX: {
            f_message = QString("%1").arg(f_message);
        }
        break;
        default: break;
    }
    // append log
    QMetaObject::invokeMethod(m_logTextArea, "append", Q_ARG(QString, f_message));
}

void LogModule::timestampToggle(const bool status) {
    m_logConfig["timestamp"] = status;
}

void LogModule::wrapToggle(const bool status) {
    m_logConfig["wrap"] = status;
}

int LogModule::heightGet() {
    return m_logConfig["height"].toInt();
}

void LogModule::heightSet(const QString &height) {
    m_logConfig["height"] = height.toInt();
    m_logTextDocument->setMaximumBlockCount(height.toInt());
}

bool LogModule::logSaveCheck() const {
    QTextDocument document;
    document.setHtml(m_logTextArea->property("text").toString());
    if (document.toPlainText().isEmpty()) {
        m_messageDialog->setProperty("title", tr("Error"));
        m_messageDialog->setProperty("text", tr("Log is empty."));
        QMetaObject::invokeMethod(m_messageDialog, "open");
        return false;
    }
    return true;
}

void LogModule::logSave(const QUrl &fileUrl) {
    const QString filePath = fileUrl.toLocalFile();
    QTextDocument document;
    document.setHtml(m_logTextArea->property("text").toString());
    if (filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << document.toPlainText();
            file.close();
            logAppend(QString("log saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), LOG_INFO);
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log saved to %2").arg(timestamp, filePath);
        } else {
            logAppend("log save failed", LOG_ERROR);
            // logging
            const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log save failed").arg(timestamp);
        }
    }
}
