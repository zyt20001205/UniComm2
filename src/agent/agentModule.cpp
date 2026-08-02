#include "agent/agentModule.h"

#include <QDateTime>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QUuid>

#include "globals.h"
#include "agent/module/mcpModule.h"
#include "agent/module/sqlModule.h"
#include "agent/module/toolsModule.h"
#include "agent/provider/bigmodelProvider.h"
#include "agent/provider/deepseekProvider.h"
#include "core/globalManager.h"
#include "document/documentModule.h"
#include "service/audio.h"

// public
AgentModule::AgentModule()
    : DockWidget("Agent"),
      m_config(g_workspaceConfig["agentConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_system("You are an IDE code assistant. "
          "When in chat mode (no tools provided), you can only answer questions. If the request cannot be handled, ask user to switch to agent mode. "
          "When in agent mode (read/write/full-access), you have access to file system, terminal, and advanced tools. "
          "Use tools first when possible. If not, consult API annotations and generate a script. "
          "When dealing with files, highly prefer using 'symbol_get' to understand the code structure and locate exactly which lines you need to use with text_get or text_set. "
          "All code must be written in English (including comments, variable names, identifiers, and strings). "
          "Use io.log() instead of print() for assistant."),
      m_conversationId(m_config["id"].toString()),
      m_conversationModel(new ConversationModel(this)),
      m_mcpModule(new McpModule(m_config["mcp"].toObject(), this)),
      m_sqlModule(new SqlModule(m_config["sql"].toObject(), this)),
      m_toolsModule(new ToolsModule(this)),
      m_bigmodelProvider(new BigmodelProvider(this)),
      m_deepseekProvider(new DeepseekProvider(this)) {
    setWidget(m_widget);

    conversationsGet();

    connect(m_toolsModule, &ToolsModule::createChat, this, &AgentModule::chatCreate);
    connect(m_toolsModule, &ToolsModule::appendChat, this, &AgentModule::chatAppend);
    connect(m_toolsModule, &ToolsModule::setState, this, &AgentModule::stateSet);
}

AgentModule::~AgentModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void AgentModule::propertySet(const QVariantHash &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_mcpMenu = qvariant_cast<QObject *>(objects["agentModuleMcpMenu"]);
    m_modeMenu = qvariant_cast<QObject *>(objects["agentModuleModeMenu"]);
    m_modelMenu = qvariant_cast<QObject *>(objects["agentModuleModelMenu"]);

    m_widget->rootContext()->setContextProperty("agentModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("renameDialog", objects["agentModuleRenameDialog"]);
    m_widget->rootContext()->setContextProperty("conversationModel", m_conversationModel);
    m_widget->rootContext()->setContextProperty("mcpMenu", m_mcpMenu);
    m_widget->rootContext()->setContextProperty("modeMenu", m_modeMenu);
    m_widget->rootContext()->setContextProperty("modelMenu", m_modelMenu);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/agent/agentModule.qml"));
    m_root = m_widget->rootObject();

    // scaffold
    connect(m_mcpModule, &McpModule::setModel, this, [this](QStandardItemModel *mcpModel) {
        m_mcpMenu->setProperty("mcpModel", QVariant::fromValue(mcpModel));
    });
    connect(m_mcpModule, &McpModule::registerTools, this, &AgentModule::toolsRegister);
    m_mcpModule->initialize();

    connect(m_toolsModule, &ToolsModule::registerTools, this, &AgentModule::toolsRegister);
    m_toolsModule->initialize();

    // base model
    connect(m_bigmodelProvider, &BigmodelProvider::setApikey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("bigmodelApikey", apikey);
    });
    connect(m_bigmodelProvider, &BigmodelProvider::setModel, this, [this](QStandardItemModel *bigmodelModel) {
        m_modelMenu->setProperty("bigmodelModel", QVariant::fromValue(bigmodelModel));
    });
    m_bigmodelProvider->apikeyGet();

    connect(m_deepseekProvider, &DeepseekProvider::setApikey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("deepseekApikey", apikey);
    });
    connect(m_deepseekProvider, &DeepseekProvider::setModel, this, [this](QStandardItemModel *deepseekModel) {
        m_modelMenu->setProperty("deepseekModel", QVariant::fromValue(deepseekModel));
    });
    m_deepseekProvider->apikeyGet();

    if (!m_conversationId.isEmpty()) {
        m_conversationComboBox->setProperty("currentValue", m_conversationId);
        conversationGet(m_conversationComboBox->property("currentValue").toString());
    }
}

void AgentModule::propertyGet(const QVariantMap &objects) {
    m_conversationComboBox = qvariant_cast<QObject *>(objects["conversationComboBox"]);
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    m_messageLabel = qvariant_cast<QObject *>(objects["messageLabel"]);
    m_modeButton = qvariant_cast<QObject *>(objects["modeButton"]);
    m_modelButton = qvariant_cast<QObject *>(objects["modelButton"]);
    m_micButton = qvariant_cast<QObject *>(objects["micButton"]);
}

void AgentModule::agentConfigSave() {
    m_config["id"] = m_conversationId;
    g_workspaceConfig["agentConfig"] = m_config;
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
            // append message
            const auto text = m_textArea->property("text").toString();
            m_turn = {
                .id = QUuid::createUuid().toString(QUuid::WithoutBraces)
            };
            chatCreate("user", text);
            conversationAppend("user", text);
            conversationSend();
        }
        break;
        case AgentState::Abort: {
            m_reply->abort();
            m_turn = {};
            conversationGet(m_conversationId);
            stateSet(AgentState::Ready);
        }
        break;
        case AgentState::Permission: {
            m_messageLabel->setProperty("message", payload.toString());
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
    if (key == "bigmodel-api-key") m_bigmodelProvider->apikeySet(apikey);
    else if (key == "deepseek-api-key") m_deepseekProvider->apikeySet(apikey);
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
    m_modeButton->setProperty("text", currentConversation.mode);
    m_modelButton->setProperty("text", currentConversation.model);
}

void AgentModule::conversationGet(const QString &id) {
    if (m_modeButton == nullptr || m_modelButton == nullptr) return;
    QMetaObject::invokeMethod(m_root, "chatClear");
    const auto [conversation, messages] = m_sqlModule->conversationGet(id);
    if (conversation.id.isEmpty()) {
        m_conversationId.clear();
        m_modeButton->setProperty("text", "");
        m_modelButton->setProperty("text", "");
        return;
    }

    m_conversationId = id;
    for (const auto &message: messages) {
        const auto &role = message.role;
        if (role == "system" || role == "tool") continue;
        const auto &content = message.content;
        if (!content.isEmpty()) chatCreate(role, content);
        // const auto toolCalls = message.value("tool_calls").toArray();
        // if (!toolCalls.isEmpty()) {
        //     for (const auto &value: toolCalls) {
        //         const auto toolCall = value.toObject();
        //         const auto function = toolCall.value("function").toObject();
        //         const auto name = function.value("name").toString();
        //         const auto arguments = function.value("arguments").toString();
        //         const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        //         const auto object = doc.object();
        //         m_toolsModule->chatCreate(name, object);
        //     }
        // }
    }
    m_modeButton->setProperty("text", conversation.mode);
    m_modelButton->setProperty("text", conversation.model);
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
    m_sqlModule->conversationAppend(id, {
                                        SqlModule::Message{
                                            .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
                                            .conversationId = id,
                                            .role = "system",
                                            .content = m_system,
                                            .createdAt = timestamp
                                        }
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

void AgentModule::conversationModeSet(const QString &mode) {
    m_sqlModule->conversationModeSet(m_conversationId, mode);
    conversationsGet();
}

void AgentModule::conversationModelSet(const QString &model) {
    m_sqlModule->conversationModelSet(m_conversationId, model);
    conversationsGet();
}

void AgentModule::conversationAppend(const QString &role, const QString &content, const QString &reasoningContent, const QString &toolCallId, const QJsonArray &toolCalls) {
    m_turn.messages.append(SqlModule::Message{
        .id = QUuid::createUuid().toString(QUuid::WithoutBraces),
        .conversationId = m_conversationId,
        .turnId = m_turn.id,
        .role = role,
        .content = content,
        .reasoningContent = reasoningContent,
        .toolCallId = toolCallId,
        .toolCalls = toolCalls,
        .createdAt = QDateTime::currentMSecsSinceEpoch()
    });
}

void AgentModule::conversationRollback() {
    const auto messages = m_sqlModule->conversationGet(m_conversationId).second;
    if (messages.size() <= 1) return;
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

void AgentModule::permissionSet(const bool status) const {
    m_toolsModule->permissionSet(status);
}

// private
void AgentModule::conversationSend() {
    auto [conversation, messages] = m_sqlModule->conversationGet(m_conversationId);
    messages.append(m_turn.messages);

    QJsonArray jsonMessages{};
    for (const auto &message: messages) {
        QJsonObject object{
            {"role", message.role},
            {"content", message.content}
        };
        if (!message.reasoningContent.isEmpty()) object["reasoning_content"] = message.reasoningContent;
        if (!message.toolCallId.isEmpty()) object["tool_call_id"] = message.toolCallId;
        if (!message.toolCalls.isEmpty()) object["tool_calls"] = message.toolCalls;
        jsonMessages.append(object);
    }

    QJsonObject body{};
    body["model"] = conversation.model;
    body["messages"] = jsonMessages;
    body["stream"] = true;
    body["tools"] = conversation.mode == "chat" ? QJsonArray{} : toolsList({"Context7"});
    QMetaObject::invokeMethod(m_textArea, "clear");
    // TODO: provider judge
    // auto *reply = g_networkAccessManager->post(m_bigmodelProvider->requestGet(), QJsonDocument(body).toJson());
    auto *reply = g_networkAccessManager->post(m_deepseekProvider->requestGet(), QJsonDocument(body).toJson());
    m_reply = reply;
    auto reasoning = std::make_shared<QString>();
    auto content = std::make_shared<QString>();
    auto reasoningId = std::make_shared<QString>();
    auto contentId = std::make_shared<QString>();
    auto toolCalls = std::make_shared<QVariantHash>();

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, reasoning, content, reasoningId, contentId, toolCalls] {
        if (reply->error() != QNetworkReply::NoError) return;
        while (reply->canReadLine()) {
            auto line = reply->readLine().trimmed();
            if (line.isEmpty()) continue;
            if (!line.startsWith("data: ")) continue;

            line = line.mid(6);
            if (line == "[DONE]") continue;

            const auto doc = QJsonDocument::fromJson(line);
            if (doc.isNull() || !doc.isObject()) continue;

            const auto choices = doc.object().value("choices").toArray();
            if (choices.isEmpty()) continue;

            const auto delta = choices.at(0).toObject().value("delta").toObject();

            const auto _reasoning = delta.value("reasoning_content").toString();
            if (!_reasoning.isEmpty()) {
                if (reasoningId->isEmpty()) {
                    stateSet(AgentState::Think);
                    *reasoningId = chatCreate("assistant", "");
                }
                reasoning->append(_reasoning);
                chatAppend(*reasoningId, _reasoning);
            }

            const auto _content = delta.value("content").toString();
            if (!_content.isEmpty()) {
                if (contentId->isEmpty()) {
                    stateSet(AgentState::Response);
                    if (!reasoningId->isEmpty()) QMetaObject::invokeMethod(m_root, "chatVisible", Q_ARG(QVariant, *reasoningId), Q_ARG(QVariant, false));
                    *contentId = chatCreate("assistant", "");
                }
                content->append(_content);
                chatAppend(*contentId, _content);
            }

            const auto _toolCalls = delta.value("tool_calls").toArray();
            if (!_toolCalls.isEmpty()) {
                QMetaObject::invokeMethod(m_root, "chatVisible", Q_ARG(QVariant, *reasoningId), Q_ARG(QVariant, false));
                for (const auto &value: _toolCalls) {
                    const auto _toolCall = value.toObject();

                    if (!_toolCall.contains("index")) continue;
                    const QString index = QString::number(_toolCall.value("index").toInt());
                    QVariantHash toolCall = toolCalls->value(index).toHash();

                    if (_toolCall.contains("id")) toolCall["id"] = _toolCall.value("id").toString();

                    const auto function = _toolCall.value("function").toObject();
                    if (function.contains("name")) toolCall["name"] = function.value("name").toString();
                    if (function.contains("arguments")) toolCall["arguments"] = toolCall.value("arguments").toString() + function.value("arguments").toString();
                    (*toolCalls)[index] = toolCall;
                }
            }
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, reasoning, content, toolCalls, mode = conversation.mode] {
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            stateSet(AgentState::Ready);
        } else if (reply->error() == QNetworkReply::NoError) {
            // tool calls
            if (!toolCalls->isEmpty()) {
                // append tool calls to m_message
                QJsonArray _toolCalls{};
                for (const auto &value: toolCalls->values()) {
                    const auto toolCall = value.toHash();
                    const auto id = toolCall.value("id").toString();
                    const auto name = toolCall.value("name").toString();
                    const auto arguments = toolCall.value("arguments").toString();

                    if (id.isEmpty() || name.isEmpty()) continue;

                    _toolCalls.append(QJsonObject{
                        {"id", id},
                        {"type", "function"},
                        {
                            "function", QJsonObject{
                                {"name", name},
                                {"arguments", arguments}
                            }
                        }
                    });
                }
                if (!_toolCalls.isEmpty()) {
                    conversationAppend("assistant", *content, *reasoning, {}, _toolCalls);
                }
                // call tools
                for (const auto &value: _toolCalls) {
                    stateSet(AgentState::Toolcall);
                    const auto toolCall = value.toObject();
                    const auto id = toolCall.value("id").toString();
                    const auto function = toolCall.value("function").toObject();
                    const auto arguments = function.value("arguments").toString();
                    const auto name = function.value("name").toString();
                    QString content{};
                    const auto owner = m_owner.value(name);
                    if (owner == "UniComm") content = m_toolsModule->toolsCall(mode, name, arguments);
                    else content = m_mcpModule->toolsCall(owner, name, arguments);
                    conversationAppend("tool", content, {}, id);
                }
                conversationSend();
            } else {
                conversationAppend("assistant", *content, *reasoning);
                m_sqlModule->conversationAppend(m_conversationId, m_turn.messages);
                m_turn = {};
                if (m_micButton->property("checked").toBool()) {
                    stateSet(AgentState::Speak, *content);
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

QString AgentModule::chatCreate(const QString &role, const QString &text) const {
    const auto messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QMetaObject::invokeMethod(m_root, "chatCreate", Q_ARG(QVariant, messageId), Q_ARG(QVariant, role), Q_ARG(QVariant, text));
    return messageId;
}

void AgentModule::chatAppend(const QString &messageId, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QVariant, messageId), Q_ARG(QVariant, text));
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
