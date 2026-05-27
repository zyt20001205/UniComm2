#ifndef UNICOMM_LLMMODULE_H
#define UNICOMM_LLMMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>
#include <QJsonObject>

class QNetworkReply;
class QNetworkAccessManager;
class QQuickWidget;
class QStandardItemModel;

class LLMTools;
class DeepseekAgent;

class LLMModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT
    Q_PROPERTY(bool active READ activeGet NOTIFY activeChanged)

public:
    explicit LLMModule();

    ~LLMModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    [[nodiscard]] bool activeGet() const {
        return m_active;
    }

    void llmConfigSave();

    Q_INVOKABLE void apikeySet(const QString &key, const QString &apikey) const;

    Q_INVOKABLE void modeSet(const QString &mode);

    Q_INVOKABLE void modelSet(const QString &model);

    Q_INVOKABLE void conversationCreate();

    Q_INVOKABLE void conversationDelete(const QString &topic);

    Q_INVOKABLE void conversationLoad(const QString &topic);

    Q_INVOKABLE void conversationStart();

    Q_INVOKABLE void conversationEnd();

    Q_INVOKABLE void permissionSet(bool status) const;

signals:
    void activeChanged();

private:
    void activeSet(bool status);

    void conversationSend();

    QString chatCreate(const QString &role, const QString &text);

    void chatAppend(const QString &id, const QString &text) const;

    void statusSet(const QString &status, const QString &text) const;

    QJsonObject m_config{};
    QString m_topic{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_modeMenu{};
    QObject *m_modelMenu{};
    QObject *m_topicComboBox{};
    QObject *m_textArea{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};

    QString m_system{};
    QStandardItemModel *m_topicStandardItemModel{};
    QHash<QString, QJsonObject> m_sessions{};

    bool m_active{};
    QNetworkReply *m_reply{};
    int m_id = 0;
    QJsonArray m_messages{};
    LLMTools *m_tools{};
    DeepseekAgent *m_deepseekAgent{};
};

#endif //UNICOMM_LLMMODULE_H
