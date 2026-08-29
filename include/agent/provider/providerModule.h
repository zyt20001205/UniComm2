#ifndef UNICOMM_PROVIDERMODULE_H
#define UNICOMM_PROVIDERMODULE_H

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QStandardItemModel>

class BaseProvider;
class ProviderModel;

class ProviderModule final : public QObject {
    Q_OBJECT

public:
    explicit ProviderModule(const QJsonObject &providers, QObject *parent = nullptr);

    ~ProviderModule() override = default;

    void propertySet(const QVariantHash &objects);

    void initialize();

    void apikeySet(const QString &provider, const QString &apikey) const;

    [[nodiscard]] BaseProvider *providerGet(const QString &id) const;

    [[nodiscard]] ProviderModel *providerModelGet() const {
        return m_providerModel;
    }

signals:
    void modelsChanged();

private:
    QObject *m_modelMenu{};
    QJsonObject m_providerConfigs{};
    ProviderModel *m_providerModel{};
    QHash<QString, BaseProvider *> m_providers{};
};

class ProviderModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        IdRole = Qt::UserRole + 1,
        ApikeyRole,
        ApiRole,
        ModelsRole
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

class ProviderModelModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        IdRole = Qt::UserRole + 1,
        ModelIdRole,
        ContextWindowRole,
        MaxOutputTokensRole
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_PROVIDERMODULE_H
