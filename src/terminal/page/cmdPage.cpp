#include "terminal/page/cmdPage.h"

// public
CmdPage::CmdPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_processName = "cmd.exe";
}
