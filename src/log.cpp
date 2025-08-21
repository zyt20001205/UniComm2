#include "../include/log.h"

#include <QPushButton>

Log::Log(QObject *parent)
    : QDockWidget("log", qobject_cast<QWidget *>(parent)) {
    auto *widget = new QWidget(); // NOLINT
    auto *layout = new QHBoxLayout(widget); // NOLINT
    setWidget(widget);

    auto *ctrlWidget = new QWidget(); // NOLINT
    layout->addWidget(ctrlWidget);
    auto *ctrlLayout = new QVBoxLayout(ctrlWidget); // NOLINT
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setAlignment(Qt::AlignTop);
    auto *timestampButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(timestampButton);
    timestampButton->setFixedSize(24, 24);
    timestampButton->setIcon(QIcon(":/icon/clock.svg"));
    timestampButton->setToolTip(tr("timestamp"));
    timestampButton->setCheckable(true);
    timestampButton->setChecked(m_logConfig["timestamp"].toBool());
    connect(timestampButton, &QPushButton::clicked, this, [this,timestampButton] {
        m_logConfig["timestamp"] = timestampButton->isChecked();
    });
    auto *saveButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(saveButton);
    saveButton->setFixedSize(24, 24);
    saveButton->setIcon(QIcon(":/icon/save.svg"));
    saveButton->setToolTip(tr("save log"));
    connect(saveButton, &QPushButton::clicked, this, &Log::logSave);
    auto *clearButton = new QPushButton(); // NOLINT
    ctrlLayout->addWidget(clearButton);
    clearButton->setFixedSize(24, 24);
    clearButton->setIcon(QIcon(":/icon/delete.svg"));
    clearButton->setToolTip(tr("clear log"));
    connect(clearButton, &QPushButton::clicked, this, &Log::logClear);

    m_textEdit = new QTextEdit();
    layout->addWidget(m_textEdit);
}

void Log::logAppend(const QString &message, const QString &level) {
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
    m_textEdit->append(f_message);
}

void Log::logConfigSave() const {
    g_config["logConfig"] = m_logConfig;
}

void Log::logSave() {
    if (m_textEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(nullptr, "Warning", tr("Log is empty."));
        return;
    }
    const QString defaultName = "log_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        tr("Save Log File"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + defaultName,
        "Plain Text (*.txt);;PDF (*.pdf);;Rich Text (*.html);;OpenDocument Text (*.odt)"
    );
    if (filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << m_textEdit->toPlainText();
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
        m_textEdit->document()->print(&printer);
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
            stream << m_textEdit->toHtml();
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
    } else {
        QTextDocumentWriter writer(filePath);
        writer.setFormat("odf");
        if (writer.write(m_textEdit->document())) {
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

void Log::logClear() const {
    m_textEdit->clear();
}
