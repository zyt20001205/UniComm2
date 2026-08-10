#include "agent/role/supervisorAgent.h"

// public
SupervisorAgent::SupervisorAgent(QObject *parent)
    : BaseAgent("supervisor", parent) {
}

bool SupervisorAgent::planRequired(const qsizetype toolCount) const {
    return toolCount >= 10;
}
