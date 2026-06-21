#include "terminal/terminalModule.h"

#include "globals.h"
#include "terminal/logModule.h"
#include "terminal/terminalPage.h"

// public
TerminalModule::TerminalModule(QWidget *parent)
    : QObject(parent)
      , m_config(g_workspaceConfig["terminalConfig"].toObject()),
      m_terminalModel(new TerminalModel(this)) {
}

TerminalModule::~TerminalModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] terminal module destructed").arg(timestamp);
}

void TerminalModule::propertySet(const QVariantHash &objects) {
    const auto &terminalMenu = qvariant_cast<QObject *>(objects["terminalModuleTerminalMenu"]);
    terminalMenu->setProperty("terminalModel", QVariant::fromValue(m_terminalModel));

    const auto terminals = m_config["terminals"].toObject();
    for (auto iterator = terminals.constBegin(); iterator != terminals.constEnd(); ++iterator) {
        const auto name = iterator.key();
        const auto command = iterator.value().toString();
        auto *item = new QStandardItem(name); // NOLINT
        item->setData(command, Qt::UserRole + 1);
        m_terminalModel->appendRow(item);
    }
    emit m_terminalModel->rowCountChanged();
}

void TerminalModule::terminalConfigSave() const {
    g_workspaceConfig["terminalConfig"] = m_config;
}

void TerminalModule::terminalOpen(const QString &name, const QString &command) {
    int index = 0;
    while (m_terminalHash.contains(index)) {
        index++;
    }
    auto *terminalPage = new TerminalPage(QString("%1 %2").arg(name, QString::number(index)), command, m_config);
    g_log->addDockWidgetAsTab(terminalPage);
    terminalPage->propertySet(QVariantHash{
    });
    m_terminalHash.insert(index, terminalPage);
    connect(terminalPage, &TerminalPage::destroyed, this, [this, index] { m_terminalHash.remove(index); });
}

// public
QHash<int, QByteArray> TerminalModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "command";
    return roles;
}
