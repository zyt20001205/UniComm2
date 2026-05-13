#ifndef UNICOMM_LLMMODULE_H
#define UNICOMM_LLMMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>

class QNetworkAccessManager;
class QQuickWidget;

class LLMModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LLMModule();

    ~LLMModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void requestSend(const QString &text = QString{});

private:
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_textArea{};
    QNetworkAccessManager *m_manager{};
    QJsonArray m_messages{};
    QJsonArray m_tools{};
};

#endif //UNICOMM_LLMMODULE_H
