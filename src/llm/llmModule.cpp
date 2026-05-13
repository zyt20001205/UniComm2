#include "llm/llmModule.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>

// public
LLMModule::LLMModule()
    : DockWidget("LLM"),
      m_widget(new QQuickWidget()),
      m_messages{
          QJsonObject{
              {"role", "system"},
              {"content", "You are a helpful assistant. Reply in plain text without any formatting."}
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

void LLMModule::requestSend() {
    QNetworkRequest request{};
    request.setUrl(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer sk-57834ede00984053bd6537822bca7960");

    m_messages.append(QJsonObject{
        {"role", "user"},
        {"content", m_textArea->property("text").toString()}
    });

    QJsonObject body{};
    body["model"] = "deepseek-chat";
    body["messages"] = m_messages;

    auto *reply = m_manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply] {
        if (reply->error() == QNetworkReply::NoError) {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            if (doc.isNull()) return;
            const auto content = doc.object()
                    .value("choices").toArray()
                    .at(0).toObject()
                    .value("message").toObject()
                    .value("content").toString();
            QMetaObject::invokeMethod(m_root, "append", Q_ARG(QVariant, content), Q_ARG(QVariant, "output"));
        } else {
            qDebug() << reply->errorString();
            qDebug() << reply->readAll();
        }
        reply->deleteLater();
    });
}
