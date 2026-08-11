#include "agent/runtime/runtimeModule.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QNetworkReply>
#include <QUuid>

#include "globals.h"
#include "agent/agentModule.h"
#include "agent/module/contextModule.h"
#include "agent/module/toolsModule.h"
#include "agent/provider/baseProvider.h"
#include "agent/provider/providerModule.h"
#include "agent/role/baseAgent.h"

// public
RuntimeModule::RuntimeModule(BaseAgent *agent, const RuntimeServices &services, QObject *parent)
    : QObject(parent),
      m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      m_agent(agent),
      m_contextModule(services.contextModule),
      m_providerModule(services.providerModule),
      m_sqlModule(services.sqlModule),
      m_toolsModule(services.toolsModule) {
    m_agent->setParent(this);
}

QString RuntimeModule::idGet() const {
    return m_id;
}

QString RuntimeModule::roleGet() const {
    return m_agent->roleGet();
}

int RuntimeModule::stateGet() const {
    return m_state;
}

QString RuntimeModule::turnIdGet() const {
    return m_turn.id;
}

void RuntimeModule::start(const QString &conversationId, const QString &text) {
    m_turn.conversationId = conversationId;
    stateSet(AgentState::Pre, text);
}

void RuntimeModule::startTask(const QString &provider, const QString &model, const int mode, const QString &task) {
    m_turn = {
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .provider = provider,
        .model = model,
        .mode = mode
    };
    const auto messageIndex = conversationAppend("user");
    auto &message = m_turn.messages[messageIndex];
    message.content = task;
    message.createdAt = QDateTime::currentMSecsSinceEpoch();
    stateSet(AgentState::Request);
}

void RuntimeModule::compact(const QString &conversationId) {
    m_turn.conversationId = conversationId;
    stateSet(AgentState::Compact);
}

void RuntimeModule::abort() {
    stateSet(AgentState::Abort);
}

void RuntimeModule::planUpdate() {
    m_turn.planned = true;
}

void RuntimeModule::permissionSet(const bool status) {
    auto &toolCall = m_turn.toolCalls[m_turn.currentTool];
    const auto &message = m_turn.messages.at(toolCall.messageIndex);
    toolCall.approved = status;
    if (toolCall.name != "plan_update" && toolCall.name != "request_user_input") emit appendChat(message.id, status ? " ✓" : " ✗");
    stateSet(AgentState::ToolExec);
}

void RuntimeModule::userInputSet(const QString &answer) {
    const auto text = answer.trimmed();
    if (text.isEmpty()) return;
    toolResultSet(text);
}

void RuntimeModule::userInputDisable() {
    m_turn.questionsAllowed = false;
    toolResultSet("The user chose not to answer and disabled further questions for this turn. Continue using your best judgment.");
}

// private
void RuntimeModule::stateSet(const int state, const QVariant &payload) {
    m_state = state;
    emit changeState();
    switch (state) {
        case AgentState::Ready: break;
        case AgentState::Error: {
            m_error = payload.toString();
            emit showError(payload.toString());
            stateSet(AgentState::Abort);
        }
        break;
        case AgentState::Pre: {
            const auto conversationId = m_turn.conversationId;
            const auto conversation = m_sqlModule->conversationGet(conversationId).first;
            if (conversation.provider.isEmpty() || conversation.model.isEmpty()) {
                stateSet(AgentState::Error, tr("Please select a model first."));
                break;
            }

            m_turn = {
                .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
                .conversationId = conversationId,
                .mode = conversation.mode
            };
            const auto messageIndex = conversationAppend("user");
            auto &message = m_turn.messages[messageIndex];
            message.content = payload.toString();
            message.createdAt = QDateTime::currentMSecsSinceEpoch();
            emit createTurn(m_turn.id, message.createdAt);
            emit createChat(m_turn.id, message.id, message.role);
            emit appendChat(message.id, message.content);

            const auto model = m_providerModule->providerGet(conversation.provider)->modelGet(conversation.model);
            stateSet(model.contextWindow > 0 && conversation.contextTokens >= model.contextWindow * 3 / 4 ? AgentState::Compact : AgentState::Request);
        }
        break;
        case AgentState::Compact: {
            const auto [conversation, history] = m_sqlModule->conversationGet(m_turn.conversationId);
            const auto [turnId, messages] = m_contextModule->compactBuild(conversation, history);
            m_turn.compactedTurnId = turnId;
            auto *provider = m_providerModule->providerGet(conversation.provider);
            const auto body = provider->requestBuild(conversation.model, messages, {}, false);
            conversationSend(provider, body);
        }
        break;
        case AgentState::Request: {
            QJsonArray context{};
            QString providerId{};
            QString modelId{};
            if (m_turn.conversationId.isEmpty()) {
                providerId = m_turn.provider;
                modelId = m_turn.model;
                context = m_contextModule->contextBuild(m_agent->systemGet(), m_turn.mode, m_turn.messages);
            } else {
                const auto [conversation, messages] = m_sqlModule->conversationGet(m_turn.conversationId);
                providerId = conversation.provider;
                modelId = conversation.model;
                context = m_contextModule->contextBuild(m_agent->systemGet(), conversation, messages, m_turn.messages);
            }
            const auto tools = m_turn.mode == AgentMode::Chat ? QJsonArray{} : m_agent->toolsGet(*m_toolsModule);
            auto *provider = m_providerModule->providerGet(providerId);
            const auto body = provider->requestBuild(modelId, context, tools, true);
            conversationSend(provider, body);
        }
        break;
        case AgentState::Abort: {
            if (m_reply != nullptr) {
                m_reply->abort();
                break;
            }
            if (!m_turn.id.isEmpty() && m_turn.conversationId.isEmpty()) {
                const auto result = m_error.isEmpty() ? QString("Agent task aborted.") : m_error;
                m_error.clear();
                m_turn = {};
                stateSet(AgentState::Ready);
                emit finishRun(result);
                break;
            }
            if (!m_turn.id.isEmpty()) {
                const auto finishedAt = QDateTime::currentMSecsSinceEpoch();
                for (auto &message: m_turn.messages) {
                    if (message.createdAt == 0) message.createdAt = finishedAt;
                    if (message.role == "assistant") emit finishChat(message.id);
                }
                m_sqlModule->conversationAppend(m_turn.conversationId, m_turn.messages, m_turn.currentUsage);
                emit finishTurn(m_turn.id, finishedAt);
            }
            m_error.clear();
            m_turn = {};
            stateSet(AgentState::Ready);
        }
        break;
        case AgentState::ToolCall: {
            auto &toolCall = m_turn.toolCalls[m_turn.currentTool];
            const auto &message = m_turn.messages.at(toolCall.messageIndex);
            const auto planUpdate = toolCall.name == "plan_update";
            const auto userInput = toolCall.name == "request_user_input";
            const auto showToolMessage = !planUpdate && !userInput;
            const auto [approved, text] = m_toolsModule->toolCall(m_turn.mode, toolCall.name, toolCall.arguments);
            toolCall.approved = approved;
            if (showToolMessage) {
                emit createChat(m_turn.id, message.id, "tool");
                emit appendChat(message.id, text);
            }

            if (!toolCall.approved) {
                stateSet(AgentState::Permission, text);
                break;
            }
            if (m_agent->roleGet() == "supervisor" && !m_turn.planned && m_agent->planRequired(m_turn.toolCount) && !planUpdate) {
                if (showToolMessage) emit appendChat(message.id, " ✗");
                toolResultSet("Plan required before further tool execution. Call plan_update first, then retry this tool.");
                break;
            }
            if (userInput && !m_turn.questionsAllowed) {
                toolResultSet("Further questions are disabled for this turn. Continue using the available context and your best judgment.");
                break;
            }
            if (userInput) {
                auto input = QJsonDocument::fromJson(toolCall.arguments.toUtf8()).object();
                auto options = input.value("options").toArray();
                while (options.size() > 3) options.removeLast();
                input["options"] = options;
                stateSet(AgentState::UserInput, input.toVariantMap());
                break;
            }
            if (showToolMessage) emit appendChat(message.id, " ✓");
            stateSet(AgentState::ToolExec);
        }
        break;
        case AgentState::Permission: {
            if (m_agent->roleGet() != "supervisor") g_agent->subagentUpdate(m_id, "Waiting for approval...");
            g_agent->permissionGet(m_id, payload.toString());
        }
        break;
        case AgentState::UserInput: {
            const auto request = payload.toMap();
            if (m_agent->roleGet() != "supervisor") g_agent->subagentUpdate(m_id, "Waiting for input...");
            g_agent->userInputGet(m_id, request);
        }
        break;
        case AgentState::ToolExec: {
            const auto toolCall = m_turn.toolCalls.at(m_turn.currentTool);
            if (!toolCall.approved) {
                toolResultSet("User denied permission to execute this tool.");
                break;
            }

            if (m_agent->roleGet() != "supervisor") g_agent->subagentUpdate(m_id, m_toolsModule->toolTextGet(toolCall.name, toolCall.arguments));
            const auto turnId = m_turn.id;
            auto future = m_toolsModule->toolExecute(m_id, toolCall.name, toolCall.arguments);
            future.then(this, [this, turnId, toolCall](const QString &result) {
                if (m_state != AgentState::ToolExec || m_turn.id != turnId) return;
                if (m_turn.toolCalls.at(m_turn.currentTool).id != toolCall.id) return;
                if (toolCall.name != "plan_update") ++m_turn.toolCount;
                toolResultSet(result);
            });
        }
        break;
        case AgentState::Speak: stateSet(AgentState::Ready);
        break;
        default: break;
    }
}

void RuntimeModule::conversationSend(const BaseProvider *provider, const QJsonObject &body) {
    auto *reply = g_networkAccessManager->post(provider->requestGet(), QJsonDocument(body).toJson());
    m_reply = reply;
    if (!body.value("stream").toBool()) {
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            if (m_reply == reply) m_reply = nullptr;
            if (reply->error() == QNetworkReply::OperationCanceledError) {
                stateSet(AgentState::Abort);
            } else if (reply->error() == QNetworkReply::NoError) {
                const auto object = QJsonDocument::fromJson(reply->readAll()).object();
                const auto summary = object.value("choices").toArray().at(0).toObject().value("message").toObject().value("content").toString();
                if (summary.isEmpty()) {
                    stateSet(AgentState::Error, "Context compact failed.");
                } else {
                    m_sqlModule->conversationCompact(m_turn.conversationId, summary, m_turn.compactedTurnId);
                    m_turn.compactedTurnId.clear();
                    emit finishCompact();
                    emit updateUsage(0);
                    stateSet(m_turn.id.isEmpty() ? AgentState::Ready : AgentState::Request);
                }
            } else {
                stateSet(AgentState::Error, reply->errorString());
            }
            reply->deleteLater();
        });
        return;
    }

    const auto assistantIndex = conversationAppend("assistant");
    const auto assistantId = m_turn.messages.at(assistantIndex).id;
    emit createChat(m_turn.id, assistantId, "assistant");
    auto toolCalls = std::make_shared<QMap<int, ToolCall>>();

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, assistantIndex, assistantId, toolCalls] {
        if (reply->error() != QNetworkReply::NoError) return;
        while (reply->canReadLine()) {
            auto line = reply->readLine().trimmed();
            if (line.isEmpty()) continue;
            if (!line.startsWith("data: ")) continue;

            line = line.mid(6);
            if (line == "[DONE]") continue;

            const auto doc = QJsonDocument::fromJson(line);
            if (doc.isNull() || !doc.isObject()) continue;

            const auto object = doc.object();
            const auto usage = object.value("usage").toObject();
            if (!usage.isEmpty()) {
                const auto promptTokens = usage.value("prompt_tokens").toInt();
                const auto completionTokens = usage.value("completion_tokens").toInt();
                const auto cacheHitTokens = usage.value("prompt_cache_hit_tokens").toInt();
                const auto reasoningTokens = usage.value("completion_tokens_details").toObject().value("reasoning_tokens").toInt();

                m_turn.currentUsage = usage.value("total_tokens").toInt();
                m_turn.usage.promptTokens += promptTokens;
                m_turn.usage.completionTokens += completionTokens;
                m_turn.usage.cacheHitTokens += cacheHitTokens;
                m_turn.usage.reasoningTokens += reasoningTokens;
                emit updateUsage(m_turn.currentUsage);
            }

            const auto choices = object.value("choices").toArray();
            if (choices.isEmpty()) continue;

            const auto delta = choices.at(0).toObject().value("delta").toObject();
            const auto reasoning = delta.value("reasoning_content").toString();
            if (!reasoning.isEmpty()) {
                auto &assistant = m_turn.messages[assistantIndex];
                if (assistant.reasoningContent.isEmpty()) stateSet(AgentState::Think);
                assistant.reasoningContent.append(reasoning);
                emit appendChatReasoning(assistantId, reasoning);
            }

            const auto content = delta.value("content").toString();
            if (!content.isEmpty()) {
                auto &assistant = m_turn.messages[assistantIndex];
                if (assistant.content.isEmpty()) stateSet(AgentState::Response);
                assistant.content.append(content);
                emit appendChat(assistantId, content);
            }

            const auto deltaToolCalls = delta.value("tool_calls").toArray();
            for (const auto &value: deltaToolCalls) {
                const auto object = value.toObject();
                if (!object.contains("index")) continue;
                const auto index = object.value("index").toInt();
                auto toolCall = toolCalls->value(index);
                if (object.contains("id")) toolCall.id = object.value("id").toString();
                const auto function = object.value("function").toObject();
                if (function.contains("name")) toolCall.name = function.value("name").toString();
                if (function.contains("arguments")) toolCall.arguments.append(function.value("arguments").toString());
                (*toolCalls)[index] = toolCall;
            }
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, assistantIndex, assistantId, toolCalls] {
        if (m_reply == reply) m_reply = nullptr;
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            stateSet(AgentState::Abort);
        } else if (reply->error() == QNetworkReply::NoError) {
            m_turn.messages[assistantIndex].createdAt = QDateTime::currentMSecsSinceEpoch();
            emit finishChat(assistantId);
            if (!toolCalls->isEmpty()) {
                QJsonArray assistantToolCalls{};
                for (auto toolCall: toolCalls->values()) {
                    if (toolCall.id.isEmpty() || toolCall.name.isEmpty()) continue;
                    assistantToolCalls.append(QJsonObject{
                        {"id", toolCall.id},
                        {"type", "function"},
                        {
                            "function", QJsonObject{
                                {"name", toolCall.name},
                                {"arguments", toolCall.arguments}
                            }
                        }
                    });
                    toolCall.messageIndex = conversationAppend("tool", toolCall.id);
                    m_turn.toolCalls.append(toolCall);
                }
                m_turn.messages[assistantIndex].toolCalls = assistantToolCalls;
                stateSet(AgentState::ToolCall);
            } else if (m_turn.conversationId.isEmpty()) {
                const auto result = m_turn.messages.at(assistantIndex).content;
                m_turn = {};
                stateSet(AgentState::Ready);
                emit finishRun(result);
            } else {
                const auto finishedAt = m_turn.messages.at(assistantIndex).createdAt;
                m_sqlModule->conversationAppend(m_turn.conversationId, m_turn.messages, m_turn.currentUsage);
                emit finishTurn(m_turn.id, finishedAt);
                m_turn = {};
                stateSet(AgentState::Ready);
            }
        } else {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            const auto message = doc.object().value("error").toObject().value("message").toString();
            stateSet(AgentState::Error, message.isEmpty() ? reply->errorString() : message);
        }
        reply->deleteLater();
    });
}

qsizetype RuntimeModule::conversationAppend(const QString &role, const QString &toolCallId) {
    const auto index = m_turn.messages.size();
    m_turn.messages.append(SqlModule::Message{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .conversationId = m_turn.conversationId,
        .turnId = m_turn.id,
        .role = role,
        .toolCallId = toolCallId
    });
    return index;
}

void RuntimeModule::toolResultSet(const QString &result) {
    const auto &toolCall = m_turn.toolCalls.at(m_turn.currentTool);
    auto &message = m_turn.messages[toolCall.messageIndex];
    message.content = result;
    message.approved = toolCall.approved;
    message.createdAt = QDateTime::currentMSecsSinceEpoch();
    ++m_turn.currentTool;
    stateSet(m_turn.currentTool < m_turn.toolCalls.size() ? AgentState::ToolCall : AgentState::Request);
}
