#include "llm/llmModule.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

#include "globals.h"
#include "document/documentModule.h"
#include "llm/llmTools.h"

// public
LLMModule::LLMModule()
    : DockWidget("LLM"),
      m_widget(new QQuickWidget()),
      m_messages{
          QJsonObject{
              {"role", "system"},
              {"content", "Use tools first when possible. If not, consult API annotations and generate a script."}
          }
      },
      m_tools{new LLMTools(this)} {
    setWidget(m_widget);
    m_manager = new QNetworkAccessManager(qApp); // NOLINT
    connect(m_tools, &LLMTools::appendChat, this, &LLMModule::chatAppend);
}

LLMModule::~LLMModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void LLMModule::propertySet(const QVariantHash &objects) {
    m_widget->rootContext()->setContextProperty("llmModule", this);
    m_widget->rootContext()->setContextProperty("global", objects["global"]);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/llm/llmModule.qml"));
    m_root = m_widget->rootObject();
}

void LLMModule::propertyGet(const QVariantMap &objects) {
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
}

void LLMModule::requestSend() {
    QNetworkRequest request{};
    request.setUrl(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const auto text = m_textArea->property("text").toString();
    if (!text.isEmpty()) {
        chatAppend("input", text, "Responding...");
        m_messages.append(QJsonObject{
            {"role", "user"},
            {"content", text}
        });
    }

    QJsonObject body{};
    body["model"] = "deepseek-chat";
    body["messages"] = m_messages;
    body["tools"] = m_tools->toolsGet();

    auto *reply = m_manager->post(request, QJsonDocument(body).toJson());

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
                    m_messages.append(QJsonObject{
                        {"role", "tool"},
                        {"tool_call_id", id},
                        {"content", m_tools->toolsSet(name, arguments)}
                    });
                }
                requestSend();
            } else {
                const auto content = message.value("content").toString();
                chatAppend("output", content, "Finished");
            }
        } else {
            const auto message = doc.object()
                    .value("error").toObject()
                    .value("message").toString();
            chatAppend("error", message, reply->errorString());
        }
        reply->deleteLater();
    });
}

void LLMModule::chatAppend(const QString& role, const QString& text, const QString &status) const {
    QMetaObject::invokeMethod(m_root, "append", Q_ARG(QVariant, role), Q_ARG(QVariant, text), Q_ARG(QVariant, status));
}
