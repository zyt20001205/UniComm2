#include "terminal/page/powershellPage.h"

// public
PowershellPage::PowershellPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_processName = "powershell.exe";
}
