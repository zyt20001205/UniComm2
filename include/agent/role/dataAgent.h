#ifndef UNICOMM_DATAAGENT_H
#define UNICOMM_DATAAGENT_H

#include "agent/role/baseAgent.h"

class DataAgent final : public BaseAgent {
public:
    explicit DataAgent(QObject *parent = nullptr);

    [[nodiscard]] QString systemGet() const override;
};

#endif //UNICOMM_DATAAGENT_H
