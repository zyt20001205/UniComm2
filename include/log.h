#ifndef LOG_H
#define LOG_H

#include <QDockWidget>
#include <QJsonObject>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include "config.h"

class Log final : public QDockWidget {
    Q_OBJECT

public:
    explicit Log(QObject *parent = nullptr);

    ~Log() override = default;

    void logAppend(const QString &message,const QString &level);

private:
    void uiInit();

    QJsonObject m_logConfig = g_config["logConfig"].toObject();

    QWidget *m_widget = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QTextEdit *m_textEdit = nullptr;
};

#endif //LOG_H
