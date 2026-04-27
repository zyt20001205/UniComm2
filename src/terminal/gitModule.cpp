#include "terminal/gitModule.h"

#include "globals.h"

// public
GitModule::GitModule(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, g_workspaceConfig["terminalConfig"].toObject()) {
    m_processName = "git.exe";
}
