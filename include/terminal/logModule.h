#ifndef UNICOMM_LOG_H
#define UNICOMM_LOG_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonObject>

class QQuickWidget;
class QTextBrowser;
class QTextDocument;
class ToastModule;

class LogModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LogModule();

    ~LogModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void logConfigSave() const;

    void logFontReload(const QJsonObject &fontConfigLog) const;

    void logFontSave(const QJsonObject &fontConfigLog);

    [[nodiscard]] QJsonArray logGet(int blockCount) const;

    void logAppend(int type, const QString &prefix, const QString &message);

    Q_INVOKABLE void timestampToggle(bool status);

    Q_INVOKABLE void wrapToggle(bool status);

    Q_INVOKABLE int heightGet();

    Q_INVOKABLE void heightSet(const QString &height);

    Q_INVOKABLE [[nodiscard]] bool logSaveCheck() const;

    Q_INVOKABLE void logSave(const QUrl &fileUrl);

    Q_INVOKABLE void linkClick(const QUrl &customUrl) const;

signals:
    void fileOpenInExplorer(const QUrl &fileUrl);

    void fileOpenInApplication(const QUrl &fileUrl);

private:
    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_textView{};
    QObject *m_textArea{};
    ToastModule *m_toast{};
    QTextDocument *m_textDocument{};
    QString m_errorFore{};
    QString m_warningFore{};
    QString m_infoFore{};
};

#endif //UNICOMM_LOG_H
