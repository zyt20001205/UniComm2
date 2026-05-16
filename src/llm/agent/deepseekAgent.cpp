#include "llm/agent/deepseekAgent.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QStandardItemModel>

#include "globals.h"

DeepseekAgent::DeepseekAgent(QObject *parent)
    : BaseAgent(parent),
      m_deepseekModel(new QStandardItemModel(this)) {
    keyGet();
}

void DeepseekAgent::modelGet() {
    m_deepseekModel->clear();
    QNetworkRequest request{};
    request.setUrl(QUrl("https://api.deepseek.com/models"));
    request.setRawHeader("Authorization", "Bearer " + m_key);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto *reply = g_networkAccessManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply] {
        const auto data = reply->readAll();
        const auto doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.object().contains("data")) {
            const auto models = doc.object().value("data").toArray();
            for (const auto &value: models) {
                const auto id = value.toObject().value("id").toString();
                m_deepseekModel->appendRow(new QStandardItem(id));
            }
        }
        reply->deleteLater();
        emit setModel(m_deepseekModel);
    });
}

void DeepseekAgent::keyGet() {
    m_request.setUrl(QUrl("https://api.deepseek.com/v1/chat/completions"));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_request.setRawHeader("Authorization", "Bearer " + m_key);
}
