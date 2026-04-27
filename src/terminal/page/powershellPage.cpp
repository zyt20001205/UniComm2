#include "terminal/page/powershellPage.h"

#include "globals.h"
#include "terminal/logModule.h"

// public
PowershellPage::PowershellPage(const QString &uniqueName, const QJsonObject &config)
    : TerminalPage(uniqueName, config) {
    m_processName = "powershell.exe";
}

// protected
void PowershellPage::closeEvent(QCloseEvent *event) {
    if (m_process->state() == QProcess::Running) {
        terminalInput("exit");
        m_process->waitForFinished();
    }
    deleteLater();
    event->accept();
}

void PowershellPage::processStart() {
    TerminalPage::processStart();
    g_log->addDockWidgetAsTab(this);
    open();
}
