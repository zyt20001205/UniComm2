#ifndef UNICOMM_AGENTMODULE_H
#define UNICOMM_AGENTMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>

#include "agent/module/sqlModule.h"

class QNetworkReply;
class QNetworkAccessManager;
class QQuickWidget;

class ConversationModel;
class McpModule;
class ToolsModule;
class BigmodelProvider;
class DeepseekProvider;

class AgentModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT
    Q_PROPERTY(int state READ stateGet WRITE stateSet NOTIFY stateChanged)

public:
    struct AgentState {
        enum {
            Ready,
            Error,
            Listen,
            STT,
            Request,
            Abort,
            Think,
            Response,
            Toolcall,
            Permission,
            Speak
        };
    };

    explicit AgentModule();

    ~AgentModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void agentConfigSave();

    [[nodiscard]] int stateGet() const {
        return m_state;
    }

    void stateSet(int state, const QVariant &payload = QVariant());

    Q_INVOKABLE void apikeySet(const QString &key, const QString &apikey) const;

    Q_INVOKABLE void conversationsGet();

    Q_INVOKABLE void conversationGet(const QString &id);

    Q_INVOKABLE void conversationInsert();

    Q_INVOKABLE void conversationRename(const QString &title);

    Q_INVOKABLE void conversationDelete();

    Q_INVOKABLE void conversationModeSet(const QString &mode);

    Q_INVOKABLE void conversationModelSet(const QString &model);

    qsizetype conversationAppend(const QString &role, const QString &toolCallId = {});

    Q_INVOKABLE void conversationRollback();

    Q_INVOKABLE void permissionSet(bool status);

signals:
    void stateChanged();

private:
    struct TurnContext {
        QString id{};
        QList<SqlModule::Message> messages{};
    };

    void conversationSend();

    void turnCreate(const QString &turnId, qint64 startedAt) const;

    void turnFinish(const QString &turnId, qint64 finishedAt) const;

    void chatCreate(const QString &turnId, const QString &messageId, const QString &role) const;

    void chatAppend(const QString &messageId, const QString &text) const;

    void chatReasoningAppend(const QString &messageId, const QString &text) const;

    void chatFinish(const QString &messageId) const;

    void toolsRegister(const QString &name, const QJsonArray &tools);

    [[nodiscard]] QJsonArray toolsList(const QStringList &names);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_mcpMenu{};
    QObject *m_modeMenu{};
    QObject *m_modelMenu{};
    QObject *m_conversationComboBox{};
    QObject *m_textArea{};
    QObject *m_messageLabel{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};
    QObject *m_micButton{};

    QString m_system{};
    QString m_conversationId{};
    QString m_permissionMessageId{};
    ConversationModel *m_conversationModel{};
    TurnContext m_turn{};

    int m_state{AgentState::Ready};
    QNetworkReply *m_reply{};
    QHash<QString, QString> m_owner{};
    QHash<QString, QJsonArray> m_tools{};
    McpModule *m_mcpModule{};
    SqlModule *m_sqlModule{};
    ToolsModule *m_toolsModule{};
    BigmodelProvider *m_bigmodelProvider{};
    DeepseekProvider *m_deepseekProvider{};
};

class ConversationModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        IdRole = Qt::UserRole + 1
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_AGE