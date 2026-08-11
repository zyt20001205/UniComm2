#ifndef UNICOMM_RUNTIMEMODULE_H
#define UNICOMM_RUNTIMEMODULE_H

#include "agent/module/sqlModule.h"

class BaseAgent;
class BaseProvider;
class ContextModule;
class ProviderModule;
class QNetworkReply;
class ToolsModule;

struct RuntimeServices {
    ContextModule *contextModule{};
    ProviderModule *providerModule{};
    SqlModule *sqlModule{};
    ToolsModule *toolsModule{};
};

class RuntimeModule final : public QObject {
    Q_OBJECT

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

    explicit RuntimeModule(BaseAgent *agent, const RuntimeServices &services, QObject *parent = nullptr);

    [[nodiscard]] QString idGet() const;

    [[nodiscard]] QString roleGet() const;

    [[nodiscard]] int stateGet() const;

    void start(const QString &conversationId, const QString &text);

    void startTask(const QString &provider, const QString &model, int mode, const QString &task);

    void compact(const QString &conversationId);

    void abort();

    void permissionSet(bool status);

    void userInputSet(const QString &answer);

    void userInputDisable();

signals:
    void changeState();

    void showError(const QString &message);

    void createTurn(const QString &turnId, qint64 startedAt);

    void finishTurn(const QString &turnId, qint64 finishedAt);

    void createChat(const QString &turnId, const QString &messageId, const QString &role);

    void appendChat(const QString &messageId, const QString &text);

    void appendChatReasoning(const QString &messageId, const QString &text);

    void finishChat(const QString &messageId);

    void requestPermission(const QString &message);

    void requestUserInput(const QVariantMap &request);

    void updatePlan(const QJsonObject &plan);

    void updateUsage(qint64 totalTokens);

    void finishCompact();

    void finishRun(const QString &result);

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
        QString conversationId{};
        QString provider{};
        QString model{};
        QString compactedTurnId{};
        int mode{AgentMode::Chat};
        QList<SqlModule::Message> messages{};
        TokenUsage usage{};
        qint64 currentUsage{};
        bool planned{false};
        bool questionsAllowed{true};
        qsizetype toolCount{};
        QList<ToolCall> toolCalls{};
        qsizetype currentTool{};
    };

    void stateSet(int state, const QVariant &payload = QVariant());

    void conversationSend(const BaseProvider *provider, const QJsonObject &body);

    qsizetype conversationAppend(const QString &role, const QString &toolCallId = {});

    void toolResultSet(const QString &result);

    QString m_id{};
    BaseAgent *m_agent{};
    QString m_error{};
    TurnContext m_turn{};
    int m_state{AgentState::Ready};
    QNetworkReply *m_reply{};
    ContextModule *m_contextModule{};
    ProviderModule *m_providerModule{};
    SqlModule *m_sqlModule{};
    ToolsModule *m_toolsModule{};
};

#endif //UNICOMM_RUNTIMEMODULE_H
