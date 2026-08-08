#include "agent/agentModule.h"

#include <QDateTime>
#include <QJsonArray>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QUuid>

#include "globals.h"
#include "agent/module/contextModule.h"
#include "agent/module/mcpModule.h"
#include "agent/module/providerModule.h"
#include "agent/module/sqlModule.h"
#include "agent/module/toolsModule.h"
#include "agent/provider/baseProvider.h"
#include "core/globalManager.h"
#include "document/documentModule.h"
#include "service/audio.h"

// public
AgentModule::AgentModule()
    : DockWidget("Agent"),
      m_config(g_workspaceConfig["agentConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_manageWindow(new QQuickView()),
      m_conversationId(m_config["id"].toString()),
      m_conversationModel(new ConversationModel(this)),
      m_contextModule(new ContextModule(this)),
      m_mcpModule(new McpModule(m_config["mcp"].toObject(), this)),
      m_providerModule(new ProviderModule(this)),
      m_sqlModule(new SqlModule(m_config["sql"].toObject(), this)),
      m_toolsModule(new ToolsModule(this)) {
    setWidget(m_widget);

    conversationsGet();
}

AgentModule::~AgentModule() {
    delete m_manageWindow;
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void AgentModule::propertySet(const QVariantHash &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_modeMenu = qvariant_cast<QObject *>(objects["agentModuleModeMenu"]);

    m_manageWindow->setTitle(tr("Agent Settings"));
    m_manageWindow->setTransientParent(g_mainWindow->windowHandle());
    m_manageWindow->rootContext()->setContextProperty("agentModule", this);
    m_manageWindow->rootContext()->setContextProperty("global", g_globalManager);
    m_manageWindow->setResizeMode(QQuickView::SizeRootObjectToView);
    m_manageWindow->setSource(QUrl("qrc:/qml/agent/agentManageWindow.qml"));

    m_widget->rootContext()->setContextProperty("agentModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_widget->rootContext()->setContextProperty("renameDialog", objects["agentModuleRenameDialog"]);
    m_widget->rootContext()->setContextProperty("conversationModel", m_conversationModel);
    m_widget->rootContext()->setContextProperty("modeMenu", m_modeMenu);
    m_widget->rootContext()->setContextProperty("modelMenu", objects["agentModuleModelMenu"]);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/agent/agentModule.qml"));
    m_root = m_widget->rootObject();

    // scaffold
    connect(m_mcpModule, &McpModule::registerTools, this, &AgentModule::toolsRegister);
    m_mcpModule->initialize();

    connect(m_toolsModule, &ToolsModule::registerTools, this, &AgentModule::toolsRegister);
    connect(m_toolsModule, &ToolsModule::updatePlan, this, [this](const QJsonObject &plan) {
        m_turn.planned = true;
        QMetaObject::invokeMethod(m_root, "planUpdate", Q_ARG(QVariant, plan.toVariantMap()));
    });
    m_toolsModule->initialize();

    m_providerModule->propertySet(QVariantHash{
        {"agentModuleModelMenu", objects["agentModuleModelMenu"]}
    });
    connect(m_providerModule, &ProviderModule::modelsChanged, this, [this] {
        if (m_conversationId.isEmpty()) return;
        modelUpdate(m_sqlModule->conversationGet(m_conversationId).first.model);
    });
    m_providerModule->initialize();

    if (!m_conversationId.isEmpty()) {
        m_conversationComboBox->setProperty("currentValue", m_conversationId);
        conversationGet(m_conversationComboBox->property("currentValue").toString());
    }
}

void AgentModule::propertyGet(const QVariantMap &objects) {
    m_conversationComboBox = qvariant_cast<QObject *>(objects["conversationComboBox"]);
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    m_messageLabel = qvariant_cast<QObject *>(objects["messageLabel"]);
    m_questionLabel = qvariant_cast<QObject *>(objects["questionLabel"]);
    m_modeButton = qvariant_cast<QObject *>(objects["modeButton"]);
    m_modelButton = qvariant_cast<QObject *>(objects["modelButton"]);
    m_micButton = qvariant_cast<QObject *>(objects["micButton"]);
}

void AgentModule::agentConfigSave() {
    m_config["id"] = m_conversationId;
    g_workspaceConfig["agentConfig"] = m_config;
}

void AgentModule::agentManage() const {
    m_manageWindow->resize(1080, 720);
    m_manageWindow->show();
}

void AgentModule::stateSet(const int state, const QVariant &payload) {
    m_state = state;
    emit stateChanged();
    switch (state) {
        case AgentState::Ready: {
            if (m_micButton->property("checked").toBool()) stateSet(AgentState::Listen);
        }
        break;
        case AgentState::Error: {
            m_messageLabel->setProperty("message", payload.toString());
            stateSet(AgentState::Ready);
        }
        break;
        case AgentState::Listen: {
            const auto pcm = g_audioService->record();
            if (pcm.isEmpty()) stateSet(AgentState::Ready);
            else stateSet(AgentState::STT, pcm);
        }
        break;
        case AgentState::STT: {
            const auto text = g_audioService->stt(payload.toByteArray());
            if (text.isEmpty()) {
                stateSet(AgentState::Ready);
            } else {
                m_textArea->setProperty("text", text);
                stateSet(AgentState::Request);
            }
        }
        break;
        case AgentState::Request: {
            if (m_turn.id.isEmpty()) {
                // get conversation
                if (m_conversationComboBox->property("currentValue").toString().isEmpty()) conversationInsert();
                // check model
                const auto conversation = m_sqlModule->conversationGet(m_conversationId).first;
                if (conversation.model.isEmpty()) {
                    m_messageDialog->setProperty("title", tr("Error"));
                    m_messageDialog->setProperty("text", tr("Please select a model first."));
                    QMetaObject::invokeMethod(m_messageDialog, "open");
                    stateSet(AgentState::Error);
                    break;
                }
                // append user message
                const auto text = m_textArea->property("text").toString();
                m_turn = {
                    .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
                    .mode = conversation.mode
                };
                const auto messageIndex = conversationAppend("user");
                auto &message = m_turn.messages[messageIndex];
                message.content = text;
                message.createdAt = QDateTime::currentMSecsSinceEpoch();
                turnCreate(m_turn.id, message.createdAt);
                chatCreate(m_turn.id, message.id, message.role);
                chatAppend(message.id, message.content);
            }
            conversationSend();
        }
        break;
        case AgentState::Abort: {
            if (m_reply != nullptr) {
                m_reply->abort();
                break;
            }
            if (!m_turn.id.isEmpty()) {
                const auto finishedAt = QDateTime::currentMSecsSinceEpoch();
                for (auto &message: m_turn.messages) {
                    if (message.createdAt == 0) message.createdAt = finishedAt;
                    if (message.role == "assistant") chatFinish(message.id);
                }
                m_sqlModule->conversationAppend(m_conversationId, m_turn.messages);
                turnFinish(m_turn.id, finishedAt);
            }
            m_turn = {};
            stateSet(AgentState::Ready);
        }
        break;
        case AgentState::ToolCall: {
            auto &toolCall = m_turn.toolCalls[m_turn.currentTool];
            const auto &message = m_turn.messages.at(toolCall.messageIndex);
            const auto owner = m_owner.value(toolCall.name);
            const auto planUpdate = toolCall.name == "plan_update";
            const auto userInput = toolCall.name == "request_user_input";
            const auto showToolMessage = owner == "UniComm" && !planUpdate && !userInput;
            QString text{};
            if (owner == "UniComm") {
                const auto [approved, toolText] = m_toolsModule->toolCall(m_turn.mode, toolCall.name, toolCall.arguments);
                toolCall.approved = approved;
                text = toolText;
                if (showToolMessage) {
                    chatCreate(m_turn.id, message.id, "tool");
                    chatAppend(message.id, text);
                }
            } else {
                toolCall.approved = true;
            }

            if (!toolCall.approved) {
                stateSet(AgentState::Permission, text);
                break;
            }
            if (!m_turn.planned && m_turn.toolCount >= 10 && !planUpdate) {
                if (showToolMessage) chatAppend(message.id, " ✗");
                toolResultSet("Plan required before further tool execution. Call plan_update first, then retry this tool.");
                break;
            }
            if (userInput && !m_turn.questionsAllowed) {
                toolResultSet("Further questions are disabled for this turn. Continue using the available context and your best judgment.");
                break;
            }
            if (userInput) {
                stateSet(AgentState::UserInput, text);
                break;
            }
            if (showToolMessage) chatAppend(message.id, " ✓");
            stateSet(AgentState::ToolExec);
        }
        break;
        case AgentState::Permission: {
            m_messageLabel->setProperty("message", payload.toString());
        }
        break;
        case AgentState::UserInput: {
            m_questionLabel->setProperty("question", payload.toString());
        }
        break;
        case AgentState::ToolExec: {
            const auto &toolCall = m_turn.toolCalls.at(m_turn.currentTool);
            QString result{};
            if (!toolCall.approved) {
                result = "User denied permission to execute this tool.";
            } else {
                const auto owner = m_owner.value(toolCall.name);
                if (owner == "UniComm") result = m_toolsModule->toolExecute(toolCall.name, toolCall.arguments);
                else result = m_mcpModule->toolsCall(owner, toolCall.name, toolCall.arguments);
                if (toolCall.name != "plan_update") ++m_turn.toolCount;
            }
            toolResultSet(result);
        }
        break;
        case AgentState::Speak: {
            g_audioService->speak(payload.toString());
            stateSet(AgentState::Ready);
        }
        break;
        default: break;
    }
}

void AgentModule::apikeySet(const QString &key, const QString &apikey) const {
    m_providerModule->apikeySet(key, apikey);
}

void AgentModule::conversationsGet() {
    const auto conversationId = m_conversationId;
    const auto conversations = m_sqlModule->conversationsGet();
    auto currentIndex = -1;
    SqlModule::Conversation currentConversation{};

    m_conversationModel->clear();
    for (const auto &conversation: conversations) {
        auto *item = new QStandardItem(conversation.title); // NOLINT
        item->setData(conversation.id, ConversationModel::IdRole);
        m_conversationModel->appendRow(item);

        if (!currentConversation.id.isEmpty() && conversation.id != conversationId) continue;
        currentIndex = m_conversationModel->rowCount() - 1;
        currentConversation = conversation;
    }

    m_conversationId = currentConversation.id;

    if (m_conversationComboBox == nullptr || m_modeButton == nullptr || m_modelButton == nullptr) return;
    m_conversationComboBox->setProperty("currentIndex", currentIndex);
    m_modeButton->setProperty("mode", currentConversation.id.isEmpty() ? -1 : currentConversation.mode);
    m_modeMenu->setProperty("selectedIndex", currentConversation.id.isEmpty() ? -1 : currentConversation.mode);
    modelUpdate(currentConversation.model);
}

void AgentModule::conversationGet(const QString &id) {
    if (m_modeButton == nullptr || m_modelButton == nullptr) return;
    QMetaObject::invokeMethod(m_root, "chatClear");
    const auto [conversation, messages] = m_sqlModule->conversationGet(id);
    if (conversation.id.isEmpty()) {
        m_conversationId.clear();
        m_modeButton->setProperty("mode", -1);
        m_modeMenu->setProperty("selectedIndex", -1);
        modelUpdate({});
        return;
    }

    m_conversationId = id;
    QString turnId{};
    qint64 finishedAt{};
    QHash<QString, ToolCall> toolCalls{};
    for (const auto &message: messages) {
        const auto &role = message.role;
        if (message.turnId != turnId) {
            if (!turnId.isEmpty()) turnFinish(turnId, finishedAt);
            turnId = message.turnId;
            turnCreate(turnId, message.createdAt);
        }
        finishedAt = message.createdAt;
        for (const auto &value: message.toolCalls) {
            const auto object = value.toObject();
            const auto function = object.value("function").toObject();
            const auto id = object.value("id").toString();
            toolCalls[id] = ToolCall{
                .id = id,
                .name = function.value("name").toString(),
                .arguments = function.value("arguments").toString()
            };
        }
        if (role == "tool") {
            const auto toolCall = toolCalls.value(message.toolCallId);
            chatCreate(turnId, message.id, role);
            chatAppend(message.id, m_toolsModule->toolTextGet(toolCall.name, toolCall.arguments));
            chatAppend(message.id, message.approved ? " ✓" : " ✗");
            chatFinish(message.id);
            continue;
        }
        const auto &content = message.content;
        if (!content.isEmpty()) {
            chatCreate(turnId, message.id, role);
            chatAppend(message.id, content);
            chatFinish(message.id);
        }
    }
    if (!turnId.isEmpty()) turnFinish(turnId, finishedAt);
    m_modeButton->setProperty("mode", conversation.mode);
    m_modeMenu->setProperty("selectedIndex", conversation.mode);
    modelUpdate(conversation.model);
    QMetaObject::invokeMethod(m_root, "followToTail", Qt::QueuedConnection);
}

void AgentModule::conversationInsert() {
    const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const auto timestamp = QDateTime::currentMSecsSinceEpoch();
    m_sqlModule->conversationInsert(SqlModule::Conversation{
        .id = id,
        .title = id,
        .createdAt = timestamp,
        .updatedAt = timestamp
    });
    m_conversationId = id;
    conversationsGet();
}

void AgentModule::conversationRename(const QString &title) {
    if (title.isEmpty()) return;
    m_sqlModule->conversationRename(m_conversationId, title);
    conversationsGet();
}

void AgentModule::conversationDelete() {
    m_sqlModule->conversationDelete(m_conversationId);
    conversationsGet();
}

void AgentModule::conversationModeSet(const int mode) {
    m_sqlModule->conversationModeSet(m_conversationId, mode);
    conversationsGet();
}

void AgentModule::conversationModelSet(const QString &id) const {
    m_sqlModule->conversationModelSet(m_conversationId, id);
    modelUpdate(id);
}

qsizetype AgentModule::conversationAppend(const QString &role, const QString &toolCallId) {
    const auto index = m_turn.messages.size();
    m_turn.messages.append(SqlModule::Message{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .conversationId = m_conversationId,
        .turnId = m_turn.id,
        .role = role,
        .toolCallId = toolCallId
    });
    return index;
}

void AgentModule::conversationRollback() {
    const auto messages = m_sqlModule->conversationGet(m_conversationId).second;
    if (messages.isEmpty()) return;
    for (auto i = messages.size() - 1; i >= 0; --i) {
        const auto &message = messages.at(i);
        if (message.role == "user") {
            m_textArea->setProperty("text", message.content);
            m_sqlModule->conversationRollback(m_conversationId, message.turnId);
            break;
        }
    }
    conversationGet(m_conversationId);
}

void AgentModule::permissionSet(const bool status) {
    auto &toolCall = m_turn.toolCalls[m_turn.currentTool];
    const auto &message = m_turn.messages.at(toolCall.messageIndex);
    toolCall.approved = status;
    if (toolCall.name != "plan_update" && toolCall.name != "request_user_input") chatAppend(message.id, status ? " ✓" : " ✗");
    stateSet(AgentState::ToolExec);
}

void AgentModule::userInputSet(const QString &answer) {
    const auto text = answer.trimmed();
    if (text.isEmpty()) return;
    toolResultSet(text);
}

void AgentModule::userInputDisable() {
    m_turn.questionsAllowed = false;
    toolResultSet("The user chose not to answer and disabled further questions for this turn. Continue using your best judgment.");
}

// private
void AgentModule::conversationSend() {
    auto [conversation, messages] = m_sqlModule->conversationGet(m_conversationId);

    QJsonObject body{};
    body["model"] = conversation.model;
    body["messages"] = m_contextModule->contextBuild(conversation.mode, messages, m_turn.messages);
    body["stream"] = true;
    body["stream_options"] = QJsonObject{{"include_usage", true}};
    body["tools"] = conversation.mode == AgentMode::Chat ? QJsonArray{} : toolsList({"Context7"});
    QMetaObject::invokeMethod(m_textArea, "clear");
    // TODO: provider judge
    auto *reply = g_networkAccessManager->post(m_providerModule->providerGet("deepseek")->requestGet(), QJsonDocument(body).toJson());
    m_reply = reply;
    const auto assistantIndex = conversationAppend("assistant");
    const auto assistantId = m_turn.messages.at(assistantIndex).id;
    chatCreate(m_turn.id, assistantId, "assistant");
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

                const QVariantMap usageMap{
                    {"currentUsage", m_turn.currentUsage},
                    {"promptTokens", m_turn.usage.promptTokens},
                    {"completionTokens", m_turn.usage.completionTokens},
                    {"cacheHitTokens", m_turn.usage.cacheHitTokens},
                    {"reasoningTokens", m_turn.usage.reasoningTokens}
                };
                QMetaObject::invokeMethod(m_root, "usageUpdate", Q_ARG(QVariant, usageMap));
            }

            const auto choices = object.value("choices").toArray();
            if (choices.isEmpty()) continue;

            const auto delta = choices.at(0).toObject().value("delta").toObject();

            const auto _reasoning = delta.value("reasoning_content").toString();
            if (!_reasoning.isEmpty()) {
                auto &assistant = m_turn.messages[assistantIndex];
                if (assistant.reasoningContent.isEmpty()) stateSet(AgentState::Think);
                assistant.reasoningContent.append(_reasoning);
                chatReasoningAppend(assistantId, _reasoning);
            }

            const auto _content = delta.value("content").toString();
            if (!_content.isEmpty()) {
                auto &assistant = m_turn.messages[assistantIndex];
                if (assistant.content.isEmpty()) stateSet(AgentState::Response);
                assistant.content.append(_content);
                chatAppend(assistantId, _content);
            }

            const auto _toolCalls = delta.value("tool_calls").toArray();
            if (!_toolCalls.isEmpty()) {
                for (const auto &value: _toolCalls) {
                    const auto _toolCall = value.toObject();

                    if (!_toolCall.contains("index")) continue;
                    const auto index = _toolCall.value("index").toInt();
                    auto toolCall = toolCalls->value(index);

                    if (_toolCall.contains("id")) toolCall.id = _toolCall.value("id").toString();

                    const auto function = _toolCall.value("function").toObject();
                    if (function.contains("name")) toolCall.name = function.value("name").toString();
                    if (function.contains("arguments")) toolCall.arguments.append(function.value("arguments").toString());
                    (*toolCalls)[index] = toolCall;
                }
            }
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, assistantIndex, assistantId, toolCalls] {
        if (m_reply == reply) m_reply = nullptr;
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            stateSet(AgentState::Abort);
        } else if (reply->error() == QNetworkReply::NoError) {
            m_turn.messages[assistantIndex].createdAt = QDateTime::currentMSecsSinceEpoch();
            chatFinish(assistantId);
            // tool calls
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
            } else {
                const auto &assistant = m_turn.messages.at(assistantIndex);
                const auto content = assistant.content;
                const auto finishedAt = assistant.createdAt;
                m_sqlModule->conversationAppend(m_conversationId, m_turn.messages);
                turnFinish(m_turn.id, finishedAt);
                m_turn = {};
                if (m_micButton->property("checked").toBool()) {
                    stateSet(AgentState::Speak, content);
                } else {
                    stateSet(AgentState::Ready);
                }
            }
        } else {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            const auto message = doc.object().value("error").toObject().value("message").toString();
            m_turn = {};
            conversationGet(m_conversationId);
            stateSet(AgentState::Error, reply->errorString());
        }
        reply->deleteLater();
    });
}

void AgentModule::turnCreate(const QString &turnId, const qint64 startedAt) const {
    QMetaObject::invokeMethod(m_root, "turnCreate", Q_ARG(QVariant, turnId), Q_ARG(QVariant, startedAt));
}

void AgentModule::turnFinish(const QString &turnId, const qint64 finishedAt) const {
    QMetaObject::invokeMethod(m_root, "turnFinish", Q_ARG(QVariant, turnId), Q_ARG(QVariant, finishedAt));
}

void AgentModule::chatCreate(const QString &turnId, const QString &messageId, const QString &role) const {
    QMetaObject::invokeMethod(m_root, "chatCreate", Q_ARG(QVariant, turnId), Q_ARG(QVariant, messageId), Q_ARG(QVariant, role));
}

void AgentModule::chatAppend(const QString &messageId, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QVariant, messageId), Q_ARG(QVariant, text));
}

void AgentModule::chatReasoningAppend(const QString &messageId, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "chatReasoningAppend", Q_ARG(QVariant, messageId), Q_ARG(QVariant, text));
}

void AgentModule::chatFinish(const QString &messageId) const {
    QMetaObject::invokeMethod(m_root, "chatFinish", Q_ARG(QVariant, messageId));
}

void AgentModule::modelUpdate(const QString &id) const {
    const auto model = m_providerModule->providerGet("deepseek")->modelGet(id);
    m_modelButton->setProperty("text", model.name);
    QMetaObject::invokeMethod(m_root, "modelUpdate", Q_ARG(QVariant, model.contextWindow));
}

void AgentModule::toolResultSet(const QString &result) {
    const auto &toolCall = m_turn.toolCalls.at(m_turn.currentTool);
    auto &message = m_turn.messages[toolCall.messageIndex];
    message.content = result;
    message.approved = toolCall.approved;
    message.createdAt = QDateTime::currentMSecsSinceEpoch();
    ++m_turn.currentTool;
    stateSet(m_turn.currentTool < m_turn.toolCalls.size() ? AgentState::ToolCall : AgentState::Request);
}

void AgentModule::toolsRegister(const QString &name, const QJsonArray &tools) {
    for (const auto &value: tools) {
        const auto _name = value.toObject().value("function").toObject().value("name").toString();
        m_owner[_name] = name;
    }
    m_tools[name] = tools;
}

QJsonArray AgentModule::toolsList(const QStringList &names) {
    auto tools = m_tools["UniComm"];
    for (const auto &name: names) {
        for (const auto &tool: m_tools.value(name)) {
            tools.append(tool);
        }
    }
    return tools;
}

// public
QHash<int, QByteArray> ConversationModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[IdRole] = "id";
    return roles;
}
