#include "llm/llmModule.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

#include "globals.h"
#include "document/documentModule.h"

// public
LLMModule::LLMModule()
    : DockWidget("LLM"),
      m_widget(new QQuickWidget()),
      m_messages{
          QJsonObject{
              {"role", "system"},
              {"content", "You are a helpful assistant. Reply in plain text without any formatting."}
          }
      },
      m_tools{
          QJsonObject{
              {"type", "function"},
              {
                  "function", QJsonObject{
                      {"name", "documentList"},
                      {"description", "Get the list of files that are currently open in the editor."},
                      {
                          "parameters", QJsonObject{
                              {"type", "object"},
                              {"properties", QJsonObject{}},
                              {"required", QJsonArray{}}
                          }
                      }
                  }
              }
          }
      } {
    setWidget(m_widget);
    m_manager = new QNetworkAccessManager(qApp); // NOLINT
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

void LLMModule::requestSend(const QString &text) {
    QNetworkRequest request{};
    request.setUrl(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer sk-57834ede00984053bd6537822bca7960");

    m_messages.append(QJsonObject{
        {"role", "user"},
        {"content", text.isEmpty() ? m_textArea->property("text").toString() : text}
    });

    QJsonObject body{};
    body["model"] = "deepseek-chat";
    body["messages"] = m_messages;
    body["tools"] = m_tools;

    auto *reply = m_manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply] {
        if (reply->error() == QNetworkReply::NoError) {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            if (doc.isNull()) return;
            const auto message = doc.object()
                    .value("choices").toArray()
                    .at(0).toObject()
                    .value("message").toObject();
            if (message.contains("tool_calls")) {
                const auto toolCalls = message.value("tool_calls").toArray();
                for (const auto &value : toolCalls) {
                    const auto toolCall = value.toObject();
                    const auto function = toolCall.value("function").toObject();
                    const auto arguments = function.value("arguments").toString();
                    const auto name = function.value("name").toString();
                    if (name == "documentList") {
                        requestSend(g_document->documentList());
                    }
                }
            } else {
                const auto content = message.value("content").toString();
                QMetaObject::invokeMethod(m_root, "append", Q_ARG(QVariant, content), Q_ARG(QVariant, "output"));
            }
        } else {
            qDebug() << reply->errorString();
            qDebug() << reply->readAll();
        }
        reply->deleteLater();
    });
}
