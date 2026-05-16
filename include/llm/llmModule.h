#ifndef UNICOMM_LLMMODULE_H
#define UNICOMM_LLMMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>
#include <QJsonObject>

class QNetworkAccessManager;
class QQuickWidget;

class LLMTools;
class DeepseekAgent;

class LLMModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit LLMModule();

    ~LLMModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void llmConfigSave();

    Q_INVOKABLE void modeSet(const QString &mode);

    Q_INVOKABLE void modelSet(const QString &model);

    Q_INVOKABLE void requestSend();

private:
    void chatAppend(const QString &role, const QString &text, const QString &status) const;

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_textArea{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};
    QObject *m_modeMenu{};
    QObject *m_modelMenu{};
    QString m_mode{};
    QString m_model{};
    QJsonArray m_messages{};
    LLMTools *m_tools{};
    DeepseekAgent *m_deepseekAgent{};
};

#endif //UNICOMM_LLMMODULE_H
