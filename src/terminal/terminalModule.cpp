#include "terminal/terminalModule.h"

#include "globals.h"
#include "terminal/page/cmdPage.h"
#include "terminal/page/powershellPage.h"

// public
TerminalModule::TerminalModule(QWidget *parent)
    : QObject(parent)
      , m_config(g_workspaceConfig["terminalConfig"].toObject()) {
}

TerminalModule::~TerminalModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] terminal module destructed").arg(timestamp);
}

void TerminalModule::propertySet(const QVariantMap &objects) {
    m_global = qvariant_cast<QObject *>(objects["global"]);
}

void TerminalModule::cmdOpen() {
    int index = 0;
    while (m_cmdHash.contains(index)) {
        index++;
    }
    auto *cmdPage = new CmdPage("Cmd " + QString::number(index), m_config);
    cmdPage->propertySet(QVariantMap{
        {"global", QVariant::fromValue(m_global)}
    });
    m_cmdHash.insert(index, cmdPage);
    connect(cmdPage, &CmdPage::destroyed, this, [this, index] { m_cmdHash.remove(index); });
}

void TerminalModule::powershellOpen() {
    int index = 0;
    while (m_powershellHash.contains(index)) {
        index++;
    }
    auto *powershellPage = new PowershellPage("Powershell " + QString::number(index), m_config);
    powershellPage->propertySet(QVariantMap{
        {"global", QVariant::fromValue(m_global)}
    });
    m_powershellHash.insert(index, powershellPage);
    connect(powershellPage, &PowershellPage::destroyed, this, [this, index] { m_powershellHash.remove(index); });
}
