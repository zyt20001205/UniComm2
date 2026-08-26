#ifndef UNICOMM_SUPERVISORAGENT_H
#define UNICOMM_SUPERVISORAGENT_H

#include "agent/role/baseAgent.h"

class SupervisorAgent final : public BaseAgent {
public:
    explicit SupervisorAgent(QObject *parent = nullptr);

    [[nodiscard]] QString systemGet() const override;

    [[nodiscard]] bool planRequired(qsizetype toolCount) const override;
};

#endif //UNICOMM_SUPERVISORAGENT_H
