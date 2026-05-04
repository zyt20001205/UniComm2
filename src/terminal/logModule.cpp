#include "terminal/logModule.h"

#include <QFileDialog>
#include <QPrinter>
#include <QQmlContext>
#include <QQuickTextDocument>
#include <QQuickWidget>
#include <QStandardPaths>

#include "globals.h"
#include "core/globalManager.h"

// public
LogModule::LogModule()
    : DockWidget("Log"),
      m_config(g_workspaceConfig["logConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_textDocument(new QTextDocument()),
      m_errorFore(g_global->dangerFore3Get()),
      m_warningFore(g_global->warningFore3Get()),
      m_infoFore(g_global->foreGet()) {
    setWidget(m_widget);
}

LogModule::~LogModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void LogModule::propertySet(const QVariantMap &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_textView = qvariant_cast<QObject *>(objects["mainWindowTextView"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textView->setProperty("font", font);

    m_widget->rootContext()->setContextProperty("logModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));
    m_widget->rootContext()->setContextProperty("heightDialog",
                                                qvariant_cast<QObject *>(objects["logModuleHeightDialog"]));
    m_widget->rootContext()->setContextProperty("linkMenu", qvariant_cast<QObject *>(objects["logModuleLinkMenu"]));

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/terminal/logModule.qml"));
}

void LogModule::propertyGet(const QVariantMap &objects) {
    // set timestamp
    auto *timestampButton = qvariant_cast<QObject *>(objects["timestampButton"]);
    timestampButton->setProperty("checked", m_config["timestamp"].toBool());
    // set wrap
    auto *wrapButton = qvariant_cast<QObject *>(objects["wrapButton"]);
    wrapButton->setProperty("checked", m_config["wrap"].toBool());
    // set font
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    const auto font = QFont(m_config["fontFamily"].toString(), m_config["fontSize"].toInt());
    m_textArea->setProperty("font", font);
    // set height
    const auto *quickTextDocument = qvariant_cast<QQuickTextDocument *>(m_textArea->property("textDocument"));
    m_textDocument = quickTextDocument->textDocument();
    m_textDocument->setMaximumBlockCount(m_config["height"].toInt());
}

void LogModule::logConfigSave() const {
    g_workspaceConfig["logConfig"] = m_config;
}

void LogModule::logFontReload(const QJsonObject &fontConfigLog) const {
    const auto logFont = QFont(fontConfigLog["fontFamily"].toString(), fontConfigLog["fontSize"].toInt());
    m_textArea->setProperty("font", logFont);
}

void LogModule::logFontSave(const QJsonObject &fontConfigLog) {
    m_config["fontFamily"] = fontConfigLog["fontFamily"].toString();
    m_config["fontSize"] = fontConfigLog["fontSize"].toInt();
}

void LogModule::logAppend(const int type, const QString &prefix, const QString &message) {
    auto _message = message;
    // check size
    const auto size = _message.size();
    if (size >= 200) {
        _message = QString::fromLatin1(
            _message.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
        _message = QString("[<a href='request.expand://reserved/%1'>%2 chars collapsed</a>]").arg(
            _message, QString::number(size));
    }
    // check level
    switch (type) {
        case LogLevel::Error: {
            _message = QString("<span style='color:%1'>%2 %3</span>").arg(m_errorFore, prefix, _message);
        }
        break;
        case LogLevel::Warning: {
            _message = QString("<span style='color:%1'>%2 %3</span>").arg(m_warningFore, prefix, _message);
        }
        break;
        case LogLevel::Info: {
            _message = QString("<span style='color:%1'>%2 %3</span>").arg(m_infoFore, prefix, _message);
        }
        break;
        case LogLevel::Transmit: {
            _message = QString("%1 %2").arg(prefix, _message);
        }
        break;
        case LogLevel::Receive: {
            _message = QString("%1 %2").arg(prefix, _message);
        }
        break;
        default: break;
    }
    // check timestamp
    if (m_config["timestamp"].toBool()) {
        const auto timestamp = QString("[%1] ").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
        _message = timestamp + _message;
    }
    QMetaObject::invokeMethod(m_textArea, "append", Q_ARG(QString, _message));
}

void LogModule::timestampToggle(const bool status) {
    m_config["timestamp"] = status;
}

void LogModule::wrapToggle(const bool status) {
    m_config["wrap"] = status;
}

int LogModule::heightGet() {
    return m_config["height"].toInt();
}

void LogModule::heightSet(const QString &height) {
    m_config["height"] = height.toInt();
    m_textDocument->setMaximumBlockCount(height.toInt());
}

bool LogModule::logSaveCheck() const {
    QTextDocument document;
    document.setHtml(m_textArea->property("text").toString());
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
    document.setHtml(m_textArea->property("text").toString());
    if (filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << document.toPlainText();
            file.close();
            logAppend(LogLevel::Info,
                      QString("log saved to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "");
            // logging
            QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log saved to %2").arg(timestamp, filePath);
        } else {
            logAppend(LogLevel::Error, "log save failed", "");
            // logging
            const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            qDebug() << QString("[%1] log save failed").arg(timestamp);
        }
    }
}

void LogModule::linkClick(const QUrl &customUrl) const {
    // qDebug() << customUrl;
    const QString scheme = customUrl.scheme();
    if (scheme == "request.expand") {
        const QStringList arguments = customUrl.path().split('/');
        const auto data = QString::fromUtf8(
            QByteArray::fromBase64(arguments[1].toLatin1(), QByteArray::Base64UrlEncoding));
        m_textView->setProperty("position", QCursor::pos());
        m_textView->setProperty("data", data);
        QMetaObject::invokeMethod(m_textView, "open");
    }
}
