#ifndef UNICOMM_HARDWAREAGENT_H
#define UNICOMM_HARDWAREAGENT_H

#include "agent/role/baseAgent.h"

class HardwareAgent final : public BaseAgent {
public:
    explicit HardwareAgent(QObject *parent = nullptr);
};

#endif //UNICOMM_HARDWAREAGENT_H
