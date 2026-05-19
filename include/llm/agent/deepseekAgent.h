#ifndef UNICOMM_DEEPSEEKAGENT_H
#define UNICOMM_DEEPSEEKAGENT_H

#include "baseAgent.h"

class DeepseekAgent final : public BaseAgent {
    Q_OBJECT

public:
    explicit DeepseekAgent(QObject *parent = nullptr);

    ~DeepseekAgent() override = default;

    void keySet(const QString &apikey) override;

    void keyGet() override;

    void modelGet() override;

private:
    QStandardItemModel *m_deepseekModel{};
};

#endif //UNICOMM_DEEPSEEKAGENT_H
