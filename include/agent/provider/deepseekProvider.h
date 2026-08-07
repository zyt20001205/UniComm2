#ifndef UNICOMM_DEEPSEEKPROVIDER_H
#define UNICOMM_DEEPSEEKPROVIDER_H

#include "baseProvider.h"

class ProviderModel;

class DeepseekProvider final : public BaseProvider {
    Q_OBJECT

public:
    explicit DeepseekProvider(QObject *parent = nullptr);

    ~DeepseekProvider() override = default;

    void apikeySet(const QString &apikey) override;

    void apikeyGet() override;

    void modelsGet() override;

    [[nodiscard]] Model modelGet(const QString &id) const override;

private:
    ProviderModel *m_deepseekModel{};
};

#endif //UNICOMM_DEEPSEEKPROVIDER_H
