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
    m_catalog = QJsonDocument::fromJson(file.readAll()).object();
    const auto providerConfigs = m_providerConfigs;
    for (auto iterator = providerConfigs.constBegin(); iterator != providerConfigs.constEnd(); ++iterator) {
        providerInsert(iterator.key(), iterator.value().toObject());
    }
}

void ProviderModule::apikeySet(const QString &provider, const QString &apikey) const {
    m_providers.value(provider)->apikeySet(apikey);
}

void ProviderModule::providerInsert(const QString &id, const QJsonObject &overrides) {
    if (m_providers.contains(id)) return;

    const auto custom = !m_catalog.contains(id);
    auto config = m_catalog.value(id).toObject();
    for (auto iterator = overrides.constBegin(); iterator != overrides.constEnd(); ++iterator) config[iterator.key()] = iterator.value();
    auto *provider = new OpenAIProvider(id, config, this); // NOLINT
    m_providerConfigs[id] = overrides;
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
    item->setData(config, ProviderModel::ConfigRole);
    item->setData(QVariant::fromValue(provider->modelListGet()), ProviderModel::ModelsRole);
    m_providerModel->appendRow(item);

    connect(provider, &BaseProvider::apikeyChanged, this, [item](const QString &apikey) {
        item->setData(apikey, ProviderModel::ApikeyRole);
    });
    connect(provider, &BaseProvider::modelsChanged, this, &ProviderModule::modelsChanged);
    provider->apikeyGet();
}

void ProviderModule::providerEdit(const QString &id, const QJsonObject &config) {
    m_providerConfigs[id] = config;

    auto *provider = static_cast<OpenAIProvider *>(m_providers.value(id));
    provider->configSet(config);
    const auto indexes = m_providerModel->match(m_providerModel->index(0, 0), ProviderModel::IdRole, id, 1, Qt::MatchExactly);
    auto *item = m_providerModel->itemFromIndex(indexes.constFirst());
    item->setText(provider->nameGet());
    item->setData(provider->baseUrlGet(), ProviderModel::BaseUrlRole);
    item->setData(provider->chatEndpointGet(), ProviderModel::ChatEndpointRole);
    item->setData(provider->modelEndpointGet(), ProviderModel::ModelEndpointRole);
    item->setData(config, ProviderModel::ConfigRole);
    provider->modelsGet();
}

void ProviderModule::providerRemove(const QString &id) {
    const auto indexes = m_providerModel->match(m_providerModel->index(0, 0), ProviderModel::IdRole, id, 1, Qt::MatchExactly);
    if (indexes.isEmpty()) return;
    const auto row = indexes.constFirst().row();
    auto *provider = m_providers.take(id);
    provider->disconnect(this);
    provider->apikeyRemove();
    provider->deleteLater();
    m_providerConfigs.remove(id);
    m_providerModel->removeRow(row);
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
    roles[ConfigRole] = "config";
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
