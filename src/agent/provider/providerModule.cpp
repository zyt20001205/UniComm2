#include "agent/provider/providerModule.h"

#include <QFile>
#include <QJsonDocument>
#include <QStandardItem>

#include "agent/provider/baseProvider.h"
#include "agent/provider/openAIProvider.h"

ProviderModule::ProviderModule(const QJsonArray &providers, QObject *parent)
    : QObject(parent),
      m_providerIds(providers),
      m_providerModel(new ProviderModel(this)) {
}

void ProviderModule::propertySet(const QVariantHash &objects) {
    m_modelMenu = qvariant_cast<QObject *>(objects["agentModuleModelMenu"]);
    m_modelMenu->setProperty("providerModel", QVariant::fromValue(m_providerModel));
}

void ProviderModule::initialize() {
    QFile file(":/config/api.json");
    if (!file.open(QIODevice::ReadOnly)) return;
    const auto catalog = QJsonDocument::fromJson(file.readAll()).object();
    for (const auto &value: m_providerIds) {
        const auto id = value.toString();
        auto *provider = new OpenAIProvider(id, catalog.value(id).toObject(), this); // NOLINT
        m_providers[id] = provider;

        auto *item = new QStandardItem(provider->nameGet()); // NOLINT
        item->setData(id, ProviderModel::IdRole);
        item->setData("", ProviderModel::ApikeyRole);
        item->setData(provider->apiGet(), ProviderModel::ApiRole);
        item->setData(QVariant::fromValue(provider->modelListGet()), ProviderModel::ModelsRole);
        m_providerModel->appendRow(item);

        connect(provider, &BaseProvider::apikeyChanged, this, [item](const QString &apikey) {
            item->setData(apikey, ProviderModel::ApikeyRole);
        });
        connect(provider, &BaseProvider::modelsChanged, this, &ProviderModule::modelsChanged);
        provider->apikeyGet();
    }
}

void ProviderModule::apikeySet(const QString &provider, const QString &apikey) const {
    m_providers.value(provider)->apikeySet(apikey);
}

BaseProvider *ProviderModule::providerGet(const QString &id) const {
    return m_providers.value(id);
}

QHash<int, QByteArray> ProviderModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[IdRole] = "id";
    roles[ApikeyRole] = "apikey";
    roles[ApiRole] = "api";
    roles[ModelsRole] = "models";
    return roles;
}

QHash<int, QByteArray> ProviderModelModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[IdRole] = "id";
    roles[ModelIdRole] = "modelId";
    roles[ContextWindowRole] = "contextWindow";
    roles[MaxOutputTokensRole] = "maxOutputTokens";
    return roles;
}
