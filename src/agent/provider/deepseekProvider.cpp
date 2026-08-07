#include "agent/provider/deepseekProvider.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <qt6keychain/keychain.h>

#include "globals.h"
#include "agent/module/providerModule.h"

DeepseekProvider::DeepseekProvider(QObject *parent)
    : BaseProvider(parent),
      m_deepseekModel(new ProviderModel(this)) {
    m_key = "deepseek-api-key";
    m_request.setUrl(QUrl("https://api.deepseek.com/v1/chat/completions"));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
}

void DeepseekProvider::apikeySet(const QString &apikey) {
    const auto job = new QKeychain::WritePasswordJob(m_service);
    job->setKey(m_key);
    job->setTextData(apikey);
    m_apikey = apikey;
    connect(job, &QKeychain::Job::finished, [this](const QKeychain::Job *j) {
        if (j->error() == QKeychain::NoError) {
            m_request.setRawHeader("Authorization", "Bearer " + QByteArray(m_apikey.toUtf8()));
            modelsGet();
        }
        emit setApikey(m_apikey);
    });
    job->start();
}

void DeepseekProvider::apikeyGet() {
    const auto job = new QKeychain::ReadPasswordJob(m_service);
    job->setKey(m_key);
    connect(job, &QKeychain::Job::finished, [this](QKeychain::Job *j) {
        if (j->error() == QKeychain::NoError) {
            const auto *readJob = static_cast<QKeychain::ReadPasswordJob *>(j);
            m_apikey = readJob->textData();
            m_request.setRawHeader("Authorization", "Bearer " + QByteArray(m_apikey.toUtf8()));
            modelsGet();
        }
        emit setApikey(m_apikey);
    });
    job->start();
}

void DeepseekProvider::modelsGet() {
    m_models.clear();
    m_deepseekModel->clear();
    QNetworkRequest request{};
    request.setUrl(QUrl("https://api.deepseek.com/models"));
    request.setRawHeader("Authorization", "Bearer " + QByteArray(m_apikey.toUtf8()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto *reply = g_networkAccessManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply] {
        const auto data = reply->readAll();
        const auto doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.object().contains("data")) {
            const auto models = doc.object().value("data").toArray();
            for (const auto &value: models) {
                const auto id = value.toObject().value("id").toString();
                const auto model = modelGet(id);
                m_models.append(model);

                auto *item = new QStandardItem(model.name); // NOLINT
                item->setData(model.id, ProviderModel::IdRole);
                item->setData(model.contextWindow, ProviderModel::ContextWindowRole);
                item->setData(model.maxOutputTokens, ProviderModel::MaxOutputTokensRole);
                m_deepseekModel->appendRow(item);
            }
        }
        reply->deleteLater();
        emit setModel(m_deepseekModel);
    });
}

BaseProvider::Model DeepseekProvider::modelGet(const QString &id) const {
    const auto object = m_catalog.value(id).toObject();
    const auto limit = object.value("limit").toObject();
    return Model{
        .id = id,
        .name = object.value("name").toString(id),
        .contextWindow = limit.value("context").toInteger(),
        .maxOutputTokens = limit.value("output").toInteger()
    };
}
