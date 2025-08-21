#ifndef LOG_H
#define LOG_H

#include <QDockWidget>
#include <QHBoxLayout>
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

    void logAppend(const QString &message, const QString &level);

private:
    QJsonObject m_logConfig = g_config["logConfig"].toObject();

    QTextEdit *m_textEdit = nullptr;

private slots:
    void logClear() const;
};

#endif //LOG_H
