#ifndef UNICOMM_DEEPSEEKAGENT_H
#define UNICOMM_DEEPSEEKAGENT_H

#include "baseAgent.h"

class DeepseekAgent final : public BaseAgent {
    Q_OBJECT

public:
    explicit DeepseekAgent(QObject *parent = nullptr);

    ~DeepseekAgent() override = default;

    void modelGet() override;

protected:
    void keyGet() override;

private:
    QStandardItemModel *m_deepseekModel{};
};

#endif //UNICOMM_DEEPSEEKAGENT_H
