#include "terminal/page/cmdPage.h"

#include "globals.h"
#include "terminal/logModule.h"

// public
CmdPage::CmdPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_name = "cmd.exe";
    m_arguments = {"/Q"};
}

// protected
void CmdPage::closeEvent(QCloseEvent *event) {
    if (m_process->state() == QProcess::Running) {
        terminalInput("exit");
        m_process->waitForFinished();
    }
    deleteLater();
    event->accept();
}

void CmdPage::processStart() {
    TerminalPage::processStart();
    g_log->addDockWidgetAsTab(this);
}
