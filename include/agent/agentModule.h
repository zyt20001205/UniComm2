#ifndef UNICOMM_AGENTMODULE_H
#define UNICOMM_AGENTMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QStandardItemModel>

#include "agent/module/sqlModule.h"

class QNetworkReply;
class QNetworkAccessManager;
class QQuickView;
class QQuickWidget;

class ConversationModel;
class ContextModule;
class McpModule;
class ProviderModule;
class ToolsModule;

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
            Pre,
            Compact,
            Request,
            Abort,
            Think,
            Response,
            ToolCall,
            Permission,
            UserInput,
            ToolExec,
            Speak
        };
    };

    struct AgentMode {
        enum {
            Chat,
            Read,
            Write,
            FullAccess
        };
    };

    explicit AgentModule();

    ~AgentModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void agentConfigSave();

    Q_INVOKABLE void agentManage() const;

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

    Q_INVOKABLE void conversationModeSet(int mode);

    Q_INVOKABLE void conversationModelSet(const QString &id);

    qsizetype conversationAppend(const QString &role, const QString &toolCallId = {});

    Q_INVOKABLE void conversationRollback();

    Q_INVOKABLE void permissionSet(bool status);

    Q_INVOKABLE void userInputSet(const QString &answer);

    Q_INVOKABLE void userInputDisable();

signals:
    void stateChanged();

private:
    struct ToolCall {
        QString id{};
        QString name{};
        QString arguments{};
        qsizetype messageIndex{-1};
        bool approved{false};
    };

    struct TokenUsage {
        qint64 promptTokens{};
        qint64 completionTokens{};
        qint64 cacheHitTokens{};
        qint64 reasoningTokens{};
    };

    struct TurnContext {
        QString id{};
        QString compactedTurnId{};
        int mode{AgentMode::Chat};
        QList<SqlModule::Message> messages{};
        TokenUsage usage{};
        qint64 currentUsage{};
        // tool
        bool planned{false};
        bool questionsAllowed{true};
        qsizetype toolCount{};
        QList<ToolCall> toolCalls{};
        qsizetype currentTool{};
    };

    void conversationSend(const QJsonObject &body);

    void turnCreate(const QString &turnId, qint64 startedAt) const;

    void turnFinish(const QString &turnId, qint64 finishedAt) const;

    void chatCreate(const QString &turnId, const QString &messageId, const QString &role) const;

    void chatAppend(const QString &messageId, const QString &text) const;

    void chatReasoningAppend(const QString &messageId, const QString &text) const;

    void chatFinish(const QString &messageId) const;

    void modelUpdate(const QString &id) const;

    void toolResultSet(const QString &result);

    void toolsRegister(const QString &name, const QJsonArray &tools);

    [[nodiscard]] QJsonArray toolsList(const QStringList &names);

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QQuickView *m_manageWindow{};
    QObject *m_root{};
    QObject *m_messageDialog{};
    QObject *m_modeMenu{};
    QObject *m_conversationComboBox{};
    QObject *m_textArea{};
    QObject *m_messageLabel{};
    QObject *m_userInputCard{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};
    QObject *m_micButton{};

    QString m_conversationId{};
    ConversationModel *m_conversationModel{};
    TurnContext m_turn{};

    int m_state{AgentState::Ready};
    QNetworkReply *m_reply{};
    QHash<QString, QString> m_owner{};
    QHash<QString, QJsonArray> m_tools{};
    ContextModule *m_contextModule{};
    McpModule *m_mcpModule{};
    ProviderModule *m_providerModule{};
    SqlModule *m_sqlModule{};
    ToolsModule *m_toolsModule{};
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
