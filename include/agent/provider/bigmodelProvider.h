#ifndef UNICOMM_BIGMODELPROVIDER_H
#define UNICOMM_BIGMODELPROVIDER_H

#include "baseProvider.h"

class ProviderModel;

class BigmodelProvider final : public BaseProvider {
    Q_OBJECT

public:
    explicit BigmodelProvider(QObject *parent = nullptr);

    ~BigmodelProvider() override = default;

    void apikeySet(const QString &apikey) override;

    void apikeyGet() override;

    void modelsGet() override;

    [[nodiscard]] Model modelGet(const QString &id) const override;

private:
    ProviderModel *m_bigmodelModel{};
};

#endif //UNICOMM_BIGMODELPROVIDER_H
