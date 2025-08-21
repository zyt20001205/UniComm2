#ifndef LOG_H
#define LOG_H

#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QMessageBox>
#include <QPrinter>
#include <QStandardPaths>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include "config.h"

class Log final : public QDockWidget {
    Q_OBJECT

public:
    explicit Log(QObject *parent = nullptr);

    ~Log() override = default;

    void logConfigSave() const;

    void logAppend(const QString &message, const QString &level);

private:
    QJsonObject m_logConfig = g_config["logConfig"].toObject();
    QTextEdit *m_textEdit = nullptr;

private slots:
    void logSave();

    void logClear() const;
};

#endif //LOG_H
