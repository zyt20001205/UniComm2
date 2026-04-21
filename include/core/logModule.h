#ifndef UNICOMM_LOG_H
#define UNICOMM_LOG_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QTextBrowser;
class QTextDocument;

class LogModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LogModule();

    ~LogModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void logConfigSave() const;

    void logFontReload(const QJsonObject &fontConfigLog) const;

    void logFontSave(const QJsonObject &fontConfigLog);

    void logAppend(const QString &message, int type);

    Q_INVOKABLE void timestampToggle(bool status);

    Q_INVOKABLE void wrapToggle(bool status);

    Q_INVOKABLE int heightGet();

    Q_INVOKABLE void heightSet(const QString &height);

    Q_INVOKABLE [[nodiscard]] bool logSaveCheck() const;

    Q_INVOKABLE void logSave(const QUrl &fileUrl);

private:
    QJsonObject m_logConfig{};
    QQuickWidget *m_logWidget{};
    QObject *m_messageDialog{};
    QObject *m_logTextArea{};
    QTextDocument *m_logTextDocument{};
};

#endif //UNICOMM_LOG_H
