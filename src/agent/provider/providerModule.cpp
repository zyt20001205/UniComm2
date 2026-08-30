#include "agent/provider/providerModule.h"

#include <QFile>
#include <QJsonDocument>
#include <QStandardItem>

#include "agent/provider/baseProvider.h"
#include "agent/provider/openAIProvider.h"

ProviderModule::ProviderModule(const QJsonObject &providers, QObject *parent)
    : QObject(parent),
      m_providerConfigs(providers),
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
    for (auto iterator = m_providerConfigs.constBegin(); iterator != m_providerConfigs.constEnd(); ++iterator) {
        const auto id = iterator.key();
        const auto custom = !catalog.contains(id);
        auto config = catalog.value(id).toObject();
        const auto overrides = iterator.value().toObject();
        for (auto override = overrides.constBegin(); override != overrides.constEnd(); ++override) config[override.key()] = override.value();
        auto *provider = new OpenAIProvider(id, config, this); // NOLINT
        m_providers[id] = provider;

        auto *item = new QStandardItem(provider->nameGet()); // NOLINT
        const auto icon = QFile::exists(":/icon/" + id + ".svg") ? "qrc:/icon/" + id + ".svg" : "qrc:/icon/model.svg";
        item->setData(QUrl(icon), Qt::DecorationRole);
        item->setData(id, ProviderModel::IdRole);
        item->setData("", ProviderModel::ApikeyRole);
        item->setData(provider->baseUrlGet(), ProviderModel::BaseUrlRole);
        item->setData(custom, ProviderModel::CustomRole);
        item->setData(provider->chatEndpointGet(), ProviderModel::ChatEndpointRole);
        item->setData(provider->modelEndpointGet(), ProviderModel::ModelEndpointRole);
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
    roles[BaseUrlRole] = "baseUrl";
    roles[CustomRole] = "custom";
    roles[ChatEndpointRole] = "chatEndpoint";
    roles[ModelEndpointRole] = "modelEndpoint";
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
