#ifndef UNICOMM_LOG_H
#define UNICOMM_LOG_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QTextBrowser;

class LogModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LogModule();

    ~LogModule() override = default;

    void logConfigSave() const;

    void logAppend(const QString &message, const QString &level);

private:
    void logSave();

    void logClear() const;

    QJsonObject m_logConfig{};
    QTextBrowser *m_logTextBrowser{};
};

#endif //UNICOMM_LOG_H
