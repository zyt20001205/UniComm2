#include "agent/role/generalAgent.h"

GeneralAgent::GeneralAgent(QObject *parent)
    : BaseAgent("general", parent) {
}

bool GeneralAgent::planRequired(const qsizetype toolCount) const {
    return toolCount >= 10;
}
