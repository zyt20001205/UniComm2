#include "terminal/terminalModule.h"

#include "globals.h"
#include "terminal/page/cmdPage.h"

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
}

void TerminalModule::cmdOpen() {
    int index = 0;
    while (m_cmdHash.contains(index)) {
        index++;
    }
    auto *cmdPage = new CmdPage("Cmd " + QString::number(index), m_config);
    cmdPage->propertySet(QVariantMap{});
    cmdPage->start();
    m_cmdHash.insert(index, cmdPage);
}
