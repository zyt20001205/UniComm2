#include "llm/provider/bigmodelProvider.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <qt6keychain/keychain.h>

#include "globals.h"

BigmodelProvider::BigmodelProvider(QObject *parent)
    : BaseProvider(parent),
      m_bigmodelModel(new QStandardItemModel(this)) {
    m_key = "bigmodel-api-key";
    m_request.setUrl(QUrl("https://open.bigmodel.cn/api/paas/v4/chat/completions"));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
}

void BigmodelProvider::apikeySet(const QString &apikey) {
    const auto job = new QKeychain::WritePasswordJob(m_service);
    job->setKey(m_key);
    job->setTextData(apikey);
    m_apikey = apikey;
    connect(job, &QKeychain::Job::finished, [this](const QKeychain::Job *j) {
        if (j->error() == QKeychain::NoError) {
            m_request.setRawHeader("Authorization", "Bearer " + QByteArray(m_apikey.toUtf8()));
            modelGet();
        }
        emit setApikey(m_apikey);
    });
    job->start();
}

void BigmodelProvider::apikeyGet() {
    const auto job = new QKeychain::ReadPasswordJob(m_service);
    job->setKey(m_key);
    connect(job, &QKeychain::Job::finished, [this](QKeychain::Job *j) {
        if (j->error() == QKeychain::NoError) {
            const auto *readJob = static_cast<QKeychain::ReadPasswordJob *>(j);
            m_apikey = readJob->textData();
            m_request.setRawHeader("Authorization", "Bearer " + QByteArray(m_apikey.toUtf8()));
            modelGet();
        }
        emit setApikey(m_apikey);
    });
    job->start();
}

void BigmodelProvider::modelGet() {
    m_bigmodelModel->clear();
    QNetworkRequest request{};
    request.setUrl(QUrl("https://open.bigmodel.cn/api/paas/v4/models"));
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
                m_bigmodelModel->appendRow(new QStandardItem(id));
            }
        }
        reply->deleteLater();
        emit setModel(m_bigmodelModel);
    });
}
