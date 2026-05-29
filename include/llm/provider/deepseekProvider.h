#ifndef UNICOMM_DEEPSEEKPROVIDER_H
#define UNICOMM_DEEPSEEKPROVIDER_H

#include "baseProvider.h"

class DeepseekProvider final : public BaseProvider {
    Q_OBJECT

public:
    explicit DeepseekProvider(QObject *parent = nullptr);

    ~DeepseekProvider() override = default;

    void apikeySet(const QString &apikey) override;

    void apikeyGet() override;

    void modelGet() override;

private:
    QStandardItemModel *m_deepseekModel{};
};

#endif //UNICOMM_DEEPSEEKPROVIDER_H
