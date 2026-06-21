#include "terminal/page/cmdPage.h"

#include "globals.h"
#include "terminal/logModule.h"

// public
CmdPage::CmdPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_name = "cmd.exe";
    m_arguments = {};
}

// protected
void CmdPage::closeEvent(QCloseEvent *event) {
    if (terminalRunning()) {
        terminalWrite("exit\r\n");
    }
    deleteLater();
    event->accept();
}

void CmdPage::processStart() {
    TerminalPage::processStart();
    g_log->addDockWidgetAsTab(this);
}
