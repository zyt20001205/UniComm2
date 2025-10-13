#ifndef UNICOMM_LOG_H
#define UNICOMM_LOG_H

#include <QDockWidget>
#include <QJsonObject>

class QTextEdit;

class LogModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit LogModule(QWidget *parent = nullptr);

    ~LogModule() override = default;

    void logConfigSave() const;

    void logAppend(const QString &message, const QString &level);

private:
    void logSave();

    void logClear() const;

    QJsonObject m_logConfig{};
    QTextEdit *m_textEdit{};
};

#endif //UNICOMM_LOG_H
