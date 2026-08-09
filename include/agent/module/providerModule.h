#ifndef UNICOMM_PROVIDERMODULE_H
#define UNICOMM_PROVIDERMODULE_H

#include <QObject>
#include <QStandardItemModel>

class BaseProvider;
class BigmodelProvider;
class DeepseekProvider;

class ProviderModule final : public QObject {
    Q_OBJECT

public:
    explicit ProviderModule(QObject *parent = nullptr);

    ~ProviderModule() override = default;

    void propertySet(const QVariantHash &objects);

    void initialize() const;

    void apikeySet(const QString &key, const QString &apikey) const;

    [[nodiscard]] BaseProvider *providerGet(const QString &id) const;

signals:
    void modelsChanged();

private:
    QObject *m_modelMenu{};
    QHash<QString, BaseProvider *> m_providers{};
    BigmodelProvider *m_bigmodelProvider{};
    DeepseekProvider *m_deepseekProvider{};
};

class ProviderModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        IdRole = Qt::UserRole + 1,
        ContextWindowRole,
        MaxOutputTokensRole
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_PROVIDERMODULE_H
