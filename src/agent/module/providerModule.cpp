#include "agent/module/providerModule.h"

#include <QFile>

#include "agent/provider/baseProvider.h"
#include "agent/provider/bigmodelProvider.h"
#include "agent/provider/deepseekProvider.h"

ProviderModule::ProviderModule(QObject *parent)
    : QObject(parent),
      m_bigmodelProvider(new BigmodelProvider(this)),
      m_deepseekProvider(new DeepseekProvider(this)) {
    m_providers = {
        {"bigmodel", m_bigmodelProvider},
        {"deepseek", m_deepseekProvider}
    };
}

void ProviderModule::propertySet(const QVariantHash &objects) {
    m_modelMenu = qvariant_cast<QObject *>(objects["agentModuleModelMenu"]);

    connect(m_bigmodelProvider, &BigmodelProvider::setApikey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("bigmodelApikey", apikey);
    });
    connect(m_bigmodelProvider, &BigmodelProvider::setModel, this, [this](QStandardItemModel *model) {
        m_modelMenu->setProperty("bigmodelModel", QVariant::fromValue(model));
    });

    connect(m_deepseekProvider, &DeepseekProvider::setApikey, this, [this](const QString &apikey) {
        m_modelMenu->setProperty("deepseekApikey", apikey);
    });
    connect(m_deepseekProvider, &DeepseekProvider::setModel, this, [this](QStandardItemModel *model) {
        m_modelMenu->setProperty("deepseekModel", QVariant::fromValue(model));
        emit modelsChanged();
    });
}

void ProviderModule::initialize() const {
    // m_bigmodelProvider->apikeyGet();

    QFile file(":/config/api.json");
    if (!file.open(QIODevice::ReadOnly)) return;
    const auto catalog = QJsonDocument::fromJson(file.readAll()).object();
    const auto provider = catalog.value("deepseek").toObject();
    m_deepseekProvider->catalogSet(provider.value("models").toObject());
    m_deepseekProvider->apikeyGet();
}

void ProviderModule::apikeySet(const QString &key, const QString &apikey) const {
    if (key == "bigmodel-api-key") m_bigmodelProvider->apikeySet(apikey);
    else if (key == "deepseek-api-key") m_deepseekProvider->apikeySet(apikey);
}

BaseProvider *ProviderModule::providerGet(const QString &id) const {
    return m_providers.value(id);
}

QHash<int, QByteArray> ProviderModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[IdRole] = "id";
    roles[ContextWindowRole] = "contextWindow";
    roles[MaxOutputTokensRole] = "maxOutputTokens";
    return roles;
}
