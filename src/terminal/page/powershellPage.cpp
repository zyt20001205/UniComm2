#include "terminal/page/powershellPage.h"

#include "globals.h"
#include "terminal/logModule.h"

// public
PowershellPage::PowershellPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_commandLine = "powershell.exe -NoLogo";
}

// protected
void PowershellPage::closeEvent(QCloseEvent *event) {
    if (terminalRunning()) {
        terminalWrite("exit\r\n");
    }
    deleteLater();
    event->accept();
}

void PowershellPage::processStart() {
    TerminalPage::processStart();
    g_log->addDockWidgetAsTab(this);
}
