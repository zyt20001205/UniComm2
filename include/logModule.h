#ifndef UNICOMM_LOG_H
#define UNICOMM_LOG_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QTextBrowser;

class LogModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LogModule();

    ~LogModule() override = default;

    void logConfigSave() const;

    void logFontReload(const QJsonObject &fontConfigLog) const;

    void logFontSave(const QJsonObject &fontConfigLog);

    void logAppend(const QString &message, const QString &level);

    Q_INVOKABLE void timestampToggle(bool status);

    Q_INVOKABLE void logSave();

private:
    QJsonObject m_logConfig{};
    QQuickWidget *m_logWidget{};
    QObject *m_logTextArea{};
    QTextBrowser *m_logTextBrowser{};
};

#endif //UNICOMM_LOG_H
