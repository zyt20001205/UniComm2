#ifndef UNICOMM_HARDWAREAGENT_H
#define UNICOMM_HARDWAREAGENT_H

#include "agent/role/baseAgent.h"

class HardwareAgent final : public BaseAgent {
public:
    explicit HardwareAgent(QObject *parent = nullptr);

    [[nodiscard]] QString systemGet() const override;
};

#endif //UNICOMM_HARDWAREAGENT_H
