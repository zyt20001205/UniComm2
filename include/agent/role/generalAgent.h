#ifndef UNICOMM_GENERALAGENT_H
#define UNICOMM_GENERALAGENT_H

#include "agent/role/baseAgent.h"

class GeneralAgent final : public BaseAgent {
public:
    explicit GeneralAgent(QObject *parent = nullptr);

    [[nodiscard]] QString systemGet() const override;

    [[nodiscard]] bool planRequired(qsizetype toolCount) const override;
};

#endif //UNICOMM_GENERALAGENT_H
