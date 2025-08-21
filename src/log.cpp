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
    // if (g_config["logConfig"].toObject()["timestamp"].toBool()) {
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString f_message = QString("[%1] %2").arg(timestamp, message);
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

void Log::logClear() const {
    m_textEdit->clear();
}
