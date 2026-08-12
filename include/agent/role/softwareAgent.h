#ifndef UNICOMM_SOFTWAREAGENT_H
#define UNICOMM_SOFTWAREAGENT_H

#include "agent/role/baseAgent.h"

class SoftwareAgent final : public BaseAgent {
public:
    explicit SoftwareAgent(QObject *parent = nullptr);
};

#endif //UNICOMM_SOFTWAREAGENT_H
