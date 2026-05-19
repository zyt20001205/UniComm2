#include "llm/llmModule.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <qt6keychain/keychain.h>

#include "globals.h"
#include "document/documentModule.h"
#include "llm/llmTools.h"
#include "llm/agent/deepseekAgent.h"

// public
LLMModule::LLMModule()
    : DockWidget("LLM"),
      m_config(g_workspaceConfig["llmConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_messages{
          QJsonObject{
              {"role", "system"},
              {
                  "content",
                  "You are an IDE code assistant. "
                  "Use tools first when possible. If not, consult API annotations and generate a script."
                  "When dealing with files, highly prefer using 'symbol_get' to understand the code structure and locate exactly which lines you need to use with text_get or text_set."
                  "All code must be written in English (including comments, variable names, identifiers, and strings)."
                  "Use io.log() instead of print() for assistant."
              }
          }
      },
      m_tools{new LLMTools(this)},
      m_deepseekAgent(new DeepseekAgent(this)) {
    setWidget(m_widget);
    connect(m_tools, &LLMTools::createChat, this, &LLMModule::chatCreate);
    connect(m_tools, &LLMTools::setStatus, this, &LLMModule::statusSet);
}

LLMModule::~LLMModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void LLMModule::propertySet(const QVariantHash &objects) {
    m_modeMenu = qvariant_cast<QObject *>(objects["llmModuleModeMenu"]);
    m_modelMenu = qvariant_cast<QObject *>(objects["llmModuleModelMenu"]);

    m_widget->rootContext()->setContextProperty("llmModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);
    m_widget->rootContext()->setContextProperty("modeMenu", m_modeMenu);
    m_widget->rootContext()->setContextProperty("modelMenu", m_modelMenu);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/llm/llmModule.qml"));
    m_root = m_widget->rootObject();

    connect(m_deepseekAgent, &DeepseekAgent::setKey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("deepseekApikey", apikey);
    });
    connect(m_deepseekAgent, &DeepseekAgent::setModel, this, [this](QStandardItemModel *agentStandardItemModel) {
        m_modelMenu->setProperty("deepseekModel", QVariant::fromValue(agentStandardItemModel));
    });
    m_deepseekAgent->apikeyGet();
}

void LLMModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    m_modeButton = qvariant_cast<QObject *>(objects["modeButton"]);
    m_modelButton = qvariant_cast<QObject *>(objects["modelButton"]);

    modeSet(m_config["mode"].toString("ask"));
    modelSet(m_config["model"].toString());
}

void LLMModule::llmConfigSave() {
    m_config["mode"] = m_mode;
    m_config["model"] = m_model;
    g_workspaceConfig["llmConfig"] = m_config;
}

void LLMModule::apikeySet(const QString &key, const QString &apikey) const {
    if (key == "deepseek-api-key") m_deepseekAgent->apikeySet(apikey);
}

void LLMModule::modeSet(const QString &mode) {
    if (mode == m_mode) return;
    m_mode = mode;
    m_modeButton->setProperty("text", mode.isEmpty() ? "ask" : mode);
    if (mode == "ask") {
        m_messages.append(QJsonObject{
            {"role", "user"},
            {"content", "[System command] Mode switched to Ask. If the request cannot be handled, ask user to switch to Agent mode."}
        });
    } else {
        m_messages.append(QJsonObject{
            {"role", "user"},
            {"content", "[System command] Mode switched to Agent. You have access to file system, terminal, and advanced tools."}
        });
    }
}

void LLMModule::modelSet(const QString &model) {
    if (model == m_model) return;
    m_model = model;
    m_modelButton->setProperty("text", model.isEmpty() ? "select" : model);
}

void LLMModule::requestSend() {
    if (m_model.isEmpty()) {
        chatCreate("error", "No model selected.");
        statusSet("error", "Please select a model first.");
        return;
    }

    const auto text = m_textArea->property("text").toString();
    if (!text.isEmpty()) {
        chatCreate("user", text);
        statusSet("user", "Responding...");
        m_messages.append(QJsonObject{
            {"role", "user"},
            {"content", text}
        });
    }
    QJsonObject body{};
    body["model"] = m_model;
    body["messages"] = m_messages;
    if (m_mode == "ask") body["stream"] = true;
    else body["tools"] = m_tools->toolsGet();
    auto *reply = g_networkAccessManager->post(m_deepseekAgent->requestGet(), QJsonDocument(body).toJson());

    // ask mode
    if (m_mode == "ask") {
        auto content = std::make_shared<QString>();
        auto reasoningId = std::make_shared<QString>();
        auto contentId = std::make_shared<QString>();
        statusSet("assistant", "Thinking...");
        connect(reply, &QNetworkReply::readyRead, this, [this, reply, content, reasoningId, contentId] {
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
                    if (reasoningId->isEmpty()) *reasoningId = chatCreate("assistant", "");
                    content->append(_reasoning);
                    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QVariant, *reasoningId), Q_ARG(QVariant, _reasoning));
                }

                const auto _content = delta.value("content").toString();
                if (!_content.isEmpty()) {
                    if (contentId->isEmpty()) *contentId = chatCreate("assistant", "");
                    content->append(_content);
                    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QVariant, *contentId), Q_ARG(QVariant, _content));
                }
            }
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, content] {
            if (reply->error() == QNetworkReply::NoError) {
                m_messages.append(QJsonObject{
                    {"role", "assistant"},
                    {"content", *content}
                });
                statusSet("assistant", "Finished");
            } else {
                const auto data = reply->readAll();
                const auto doc = QJsonDocument::fromJson(data);
                const auto message = doc.object().value("error").toObject().value("message").toString();
                chatCreate("error", message);
                statusSet("error", reply->errorString());
            }
            reply->deleteLater();
        });
    }
    // agent mode
    // TODO: agent stream output
    else {
        connect(reply, &QNetworkReply::finished, [this, reply] {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            if (doc.isNull()) return;
            if (reply->error() == QNetworkReply::NoError) {
                const auto message = doc.object()
                        .value("choices").toArray()
                        .at(0).toObject()
                        .value("message").toObject();
                m_messages.append(message);
                if (message.contains("tool_calls")) {
                    const auto toolCalls = message.value("tool_calls").toArray();
                    for (const auto &value: toolCalls) {
                        const auto toolCall = value.toObject();
                        const auto id = toolCall.value("id").toString();
                        const auto function = toolCall.value("function").toObject();
                        const auto arguments = function.value("arguments").toString();
                        const auto name = function.value("name").toString();
                        const auto content = m_tools->toolsSet(m_mode, name, arguments);
                        m_messages.append(QJsonObject{
                            {"role", "tool"},
                            {"tool_call_id", id},
                            {"content", content}
                        });
                    }
                    requestSend();
                } else {
                    const auto content = message.value("content").toString();
                    chatCreate("assistant", content);
                    statusSet("assistant", "Finished");
                }
            } else {
                const auto message = doc.object()
                        .value("error").toObject()
                        .value("message").toString();
                chatCreate("user", message);
                statusSet("user", reply->errorString());
            }
            reply->deleteLater();
        });
    }
}

void LLMModule::permissionSet(const bool status) const {
    m_tools->permissionSet(status);
}

// private
QString LLMModule::chatCreate(const QString &role, const QString &text) {
    const auto id = "id_" + QString::number(m_id++);
    QMetaObject::invokeMethod(m_root, "chatCreate",Q_ARG(QVariant, id),  Q_ARG(QVariant, role), Q_ARG(QVariant, text));
    return id;
}

void LLMModule::statusSet(const QString &role, const QString &status) const {
    QMetaObject::invokeMethod(m_root, "statusSet", Q_ARG(QVariant, role), Q_ARG(QVariant, status));
}
