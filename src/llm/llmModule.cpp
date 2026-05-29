#include "llm/llmModule.h"

#include <QDir>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"
#include "document/documentModule.h"
#include "llm/module/toolsModule.h"
#include "llm/provider/deepseekProvider.h"

// public
LLMModule::LLMModule()
    : DockWidget("LLM"),
      m_config(g_workspaceConfig["llmConfig"].toObject()),
      m_topic(m_config["topic"].toString()),
      m_widget(new QQuickWidget()),
      m_system("You are an IDE code assistant. "
          "When in ask mode (no tools provided), you can only answer questions. If the request cannot be handled, ask user to switch to Agent mode. "
          "When in agent mode (tools provided), you have access to file system, terminal, and advanced tools. "
          "Use tools first when possible. If not, consult API annotations and generate a script. "
          "When dealing with files, highly prefer using 'symbol_get' to understand the code structure and locate exactly which lines you need to use with text_get or text_set. "
          "All code must be written in English (including comments, variable names, identifiers, and strings). "
          "Use io.log() instead of print() for assistant."),
      m_topicStandardItemModel(new QStandardItemModel(this)),
      m_tools(new ToolsModule(this)),
      m_deepseekProvider(new DeepseekProvider(this)) {
    setWidget(m_widget);

    const auto dirPath = QDir(g_workspaceUrl.toLocalFile()).filePath("llm");
    for (const auto &value: QDir(dirPath).entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        auto sessionFile = QFile(value.absoluteFilePath());
        const auto topic = value.baseName();
        if (sessionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const auto sessionDoc = QJsonDocument::fromJson(sessionFile.readAll());
            sessionFile.close();
            if (sessionDoc.isNull() || !sessionDoc.isObject()) continue;
            if (sessionDoc.object().isEmpty()) continue;
            m_topicStandardItemModel->appendRow(new QStandardItem(topic));
            m_sessions[topic] = sessionDoc.object();
        }
    }

    connect(m_tools, &ToolsModule::createChat, this, &LLMModule::chatCreate);
    connect(m_tools, &ToolsModule::appendChat, this, &LLMModule::chatAppend);
    connect(m_tools, &ToolsModule::setStatus, this, &LLMModule::statusSet);
}

LLMModule::~LLMModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void LLMModule::propertySet(const QVariantHash &objects) {
    m_messageDialog = qvariant_cast<QObject *>(objects["mainWindowMessageDialog"]);
    m_modeMenu = qvariant_cast<QObject *>(objects["llmModuleModeMenu"]);
    m_modelMenu = qvariant_cast<QObject *>(objects["llmModuleModelMenu"]);

    m_widget->rootContext()->setContextProperty("llmModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("renameDialog", objects["llmModuleRenameDialog"]);
    m_widget->rootContext()->setContextProperty("topicStandardItemModel", m_topicStandardItemModel);
    m_widget->rootContext()->setContextProperty("modeMenu", m_modeMenu);
    m_widget->rootContext()->setContextProperty("modelMenu", m_modelMenu);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/llm/llmModule.qml"));
    m_root = m_widget->rootObject();

    connect(m_deepseekProvider, &DeepseekProvider::setApikey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("deepseekApikey", apikey);
    });
    connect(m_deepseekProvider, &DeepseekProvider::setModel, this, [this](QStandardItemModel *agentStandardItemModel) {
        m_modelMenu->setProperty("deepseekModel", QVariant::fromValue(agentStandardItemModel));
    });
    m_deepseekProvider->apikeyGet();

    conversationLoad(m_topic);
}

void LLMModule::propertyGet(const QVariantMap &objects) {
    m_topicComboBox = qvariant_cast<QObject *>(objects["topicComboBox"]);
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    m_modeButton = qvariant_cast<QObject *>(objects["modeButton"]);
    m_modelButton = qvariant_cast<QObject *>(objects["modelButton"]);
}

void LLMModule::llmConfigSave() {
    m_config["topic"] = m_topic;
    g_workspaceConfig["llmConfig"] = m_config;

    const auto dirPath = QDir(g_workspaceUrl.toLocalFile()).filePath("llm");
    for (const auto &value: QDir(dirPath).entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        QFile::remove(value.absoluteFilePath());
    }
    for (auto it = m_sessions.constBegin(); it != m_sessions.constEnd(); ++it) {
        const auto &topic = it.key();
        const auto sessionPath = QDir(dirPath).filePath(topic + ".json");
        auto sessionFile = QFile(sessionPath);
        if (sessionFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            const auto sessionDoc = QJsonDocument(it.value());
            sessionFile.write(sessionDoc.toJson(QJsonDocument::Indented));
            sessionFile.close();
        }
    }
}

void LLMModule::apikeySet(const QString &key, const QString &apikey) const {
    if (key == "deepseek-api-key") m_deepseekProvider->apikeySet(apikey);
}

void LLMModule::modeSet(const QString &mode) {
    if (m_sessions[m_topic]["mode"].toString() == mode) return;
    m_sessions[m_topic]["mode"] = mode;
    m_modeButton->setProperty("text", m_sessions[m_topic]["mode"].toString());
}

void LLMModule::modelSet(const QString &model) {
    if (m_sessions[m_topic]["model"].toString() == model) return;
    m_sessions[m_topic]["model"] = model;
    m_modelButton->setProperty("text", m_sessions[m_topic]["model"].toString());
}

void LLMModule::conversationRename(const QString &oldTopic, const QString &newTopic) {
    const auto session = m_sessions.take(oldTopic);
    m_sessions[newTopic] = session;
    for (int row = 0; row < m_topicStandardItemModel->rowCount(); ++row) {
        const auto item = m_topicStandardItemModel->item(row, 0);
        if (item->text() == oldTopic) {
            item->setText(newTopic);
            break;
        }
    }
}

void LLMModule::conversationCreate() {
    const auto topic = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_topicStandardItemModel->appendRow(new QStandardItem(topic));
    m_topicComboBox->setProperty("currentValue", topic);
    m_sessions[topic] = QJsonObject{
        {"mode", ""},
        {"model", ""},
        {
            "messages", QJsonArray{
                QJsonObject{
                    {"role", "system"},
                    {"content", m_system}
                }
            }
        }
    };
}

void LLMModule::conversationDelete(const QString &topic) {
    for (int row = 0; row < m_topicStandardItemModel->rowCount(); ++row) {
        if (m_topicStandardItemModel->item(row, 0)->text() == topic) {
            m_topicStandardItemModel->removeRow(row);
            break;
        }
    }
    m_sessions.remove(topic);
}

void LLMModule::conversationLoad(const QString &topic) {
    if (topic.isEmpty() || m_modeButton == nullptr || m_modelButton == nullptr) return;
    m_topic = topic;
    const auto session = m_sessions[m_topic];
    chatClear();
    for (const auto &value: session["messages"].toArray()) {
        const auto message = value.toObject();
        const auto role = message.value("role").toString();
        if (role == "system" || role == "tool") continue;
        const auto content = message.value("content").toString();
        if (!content.isEmpty()) chatCreate(role, content);
        const auto toolCalls = message.value("tool_calls").toArray();
        if (!toolCalls.isEmpty()) {
            for (const auto &value: toolCalls) {
                const auto toolCall = value.toObject();
                const auto function = toolCall.value("function").toObject();
                const auto name = function.value("name").toString();
                const auto arguments = function.value("arguments").toString();
                const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
                const auto object = doc.object();
                m_tools->chatCreate(name, object);
            }
        }
    }
    m_modeButton->setProperty("text", session["mode"].toString());
    m_modelButton->setProperty("text", session["model"].toString());
}

void LLMModule::conversationUndo() {
    auto messages = m_sessions[m_topic]["messages"].toArray();
    if (messages.size() <= 1) return;
    for (auto i = messages.size() - 1; i >= 0; --i) {
        const auto message = messages.takeAt(i).toObject();
        if (message.value("role").toString() == "user") {
            const auto content = message["content"].toString();
            m_textArea->setProperty("text", content);
            break;
        }
    }
    m_sessions[m_topic]["messages"] = messages;
    qDebug() << m_sessions[m_topic]["messages"];
    conversationLoad(m_topic);
}

void LLMModule::conversationStart() {
    // get topic
    if (m_topicComboBox->property("currentText").toString().isEmpty()) conversationCreate();
    m_topic = m_topicComboBox->property("currentText").toString();
    // check model
    if (m_sessions[m_topic]["model"].toString().isEmpty()) {
        m_messageDialog->setProperty("title", tr("Error"));
        m_messageDialog->setProperty("text", tr("Please select a model first."));
        QMetaObject::invokeMethod(m_messageDialog, "open");
        return;
    }
    // append message
    const auto text = m_textArea->property("text").toString();
    if (!text.isEmpty()) {
        chatCreate("user", text);
        auto messages = m_sessions[m_topic]["messages"].toArray();
        messages.append(QJsonObject{
            {"role", "user"},
            {"content", text}
        });
        m_sessions[m_topic]["messages"] = messages;
    }

    activeSet(true);
    conversationSend();
}

void LLMModule::conversationEnd() {
    activeSet(false);
    m_reply->abort();
}

void LLMModule::permissionSet(const bool status) const {
    m_tools->permissionSet(status);
}

// private
void LLMModule::activeSet(const bool status) {
    if (m_active == status) return;
    m_active = status;
    emit activeChanged();
}

void LLMModule::conversationSend() {
    const auto session = m_sessions[m_topic];
    QJsonObject body{};
    body["model"] = session["model"];
    body["messages"] = session["messages"];
    body["stream"] = true;
    body["tools"] = session["mode"] == "ask" ? QJsonArray{} : m_tools->toolsGet();
    QMetaObject::invokeMethod(m_textArea, "clear");
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
                    *reasoningId = chatCreate("assistant", "");
                    statusSet("busy", "Thinking...");
                }
                reasoning->append(_reasoning);
                chatAppend(*reasoningId, _reasoning);
            }

            const auto _content = delta.value("content").toString();
            if (!_content.isEmpty()) {
                if (contentId->isEmpty()) {
                    if (!reasoningId->isEmpty()) QMetaObject::invokeMethod(m_root, "chatVisible", Q_ARG(QVariant, *reasoningId), Q_ARG(QVariant, false));
                    *contentId = chatCreate("assistant", "");
                    statusSet("busy", "Responding...");
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
    connect(reply, &QNetworkReply::finished, this, [this, reply, reasoning, content, toolCalls] {
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            activeSet(false);
            statusSet("idle", "Ready");
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
                    auto messages = m_sessions[m_topic]["messages"].toArray();
                    messages.append(QJsonObject{
                        {"role", "assistant"},
                        {"content", *content},
                        {"reasoning_content", *reasoning},
                        {"tool_calls", _toolCalls}
                    });
                    m_sessions[m_topic]["messages"] = messages;
                }
                // call tools
                for (const auto &value: _toolCalls) {
                    const auto toolCall = value.toObject();
                    const auto id = toolCall.value("id").toString();
                    const auto function = toolCall.value("function").toObject();
                    const auto arguments = function.value("arguments").toString();
                    const auto name = function.value("name").toString();
                    const auto content = m_tools->toolsSet(m_sessions[m_topic]["mode"].toString(), name, arguments);
                    auto messages = m_sessions[m_topic]["messages"].toArray();
                    messages.append(QJsonObject{
                        {"role", "tool"},
                        {"tool_call_id", id},
                        {"content", content},
                    });
                    m_sessions[m_topic]["messages"] = messages;
                }
                conversationSend();
            } else {
                auto messages = m_sessions[m_topic]["messages"].toArray();
                messages.append(QJsonObject{
                    {"role", "assistant"},
                    {"content", *content}
                });
                m_sessions[m_topic]["messages"] = messages;
                activeSet(false);
                statusSet("idle", "Ready");
            }
        } else {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            const auto message = doc.object().value("error").toObject().value("message").toString();
            activeSet(false);
            statusSet("error", reply->errorString());
        }
        reply->deleteLater();
    });
}

void LLMModule::chatClear() {
    QMetaObject::invokeMethod(m_root, "chatClear");
}

QString LLMModule::chatCreate(const QString &role, const QString &text) {
    const auto messageId = "id_" + QString::number(m_id++);
    QMetaObject::invokeMethod(m_root, "chatCreate", Q_ARG(QVariant, messageId), Q_ARG(QVariant, role), Q_ARG(QVariant, text));
    return messageId;
}

void LLMModule::chatAppend(const QString &messageId, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QVariant, messageId), Q_ARG(QVariant, text));
}

void LLMModule::statusSet(const QString &status, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "statusSet", Q_ARG(QVariant, status), Q_ARG(QVariant, text));
}
