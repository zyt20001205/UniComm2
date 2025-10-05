#ifndef LOG_H
#define LOG_H

#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonObject>
#include <QMessageBox>
#include <QPrinter>
#include <QStandardPaths>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

extern QJsonObject g_config;

class Log final : public QDockWidget {
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);

    ~Log() override = default;

    void logConfigSave() const;

    void logAppend(const QString &message, const QString &level);

private:
    void logSave();

    void logClear() const;

    QJsonObject m_logConfig = g_config["logConfig"].toObject();
    QTextEdit *m_textEdit = nullptr;
};

#endif //LOG_H
