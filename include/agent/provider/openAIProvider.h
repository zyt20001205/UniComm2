#ifndef UNICOMM_OPENAIPROVIDER_H
#define UNICOMM_OPENAIPROVIDER_H

#include <QList>
#include <QNetworkRequest>
#include <QUrl>

#include "baseProvider.h"

class ProviderModelModel;

class OpenAIProvider final : public BaseProvider {
    Q_OBJECT

public:
    explicit OpenAIProvider(const QString &id, const QJsonObject &provider, QObject *parent = nullptr);

    ~OpenAIProvider() override = default;

    [[nodiscard]] QString idGet() const {
        return m_id;
    }

    [[nodiscard]] QString nameGet() const {
        return m_name;
    }

    [[nodiscard]] QUrl apiGet() const {
        return m_api;
    }

    [[nodiscard]] ProviderModelModel *modelListGet() const {
        return m_modelList;
    }

    [[nodiscard]] QNetworkRequest requestGet() const override {
        return m_request;
    }

    [[nodiscard]] QJsonObject requestBuild(const QString &model, const QJsonArray &messages, const QJsonArray &tools, bool stream) const override;

    void apikeyGet() override;

    void apikeySet(const QString &apikey) override;

    void modelsGet() override;

    [[nodiscard]] Model modelGet(const QString &id) const override;

private:
    QString m_id{};
    QString m_name{};
    QUrl m_api{};
    QList<Model> m_models{};
    QNetworkRequest m_request{};
    ProviderModelModel *m_modelList{};
};

#endif //UNICOMM_OPENAIPROVIDER_H
