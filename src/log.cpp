#include "../include/log.h"

Log::Log(QObject *parent)
    : QDockWidget("log", qobject_cast<QWidget*>(parent)) {
    uiInit();
}

void Log::uiInit() {
    m_widget = new QWidget();
    m_layout = new QVBoxLayout(m_widget);
    setWidget(m_widget);

    m_textEdit = new QTextEdit();
    m_layout->addWidget(m_textEdit);
}

void Log::logAppend(const QString &message,const QString &level) {
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
