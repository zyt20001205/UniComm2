#include "terminal/terminalModule.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQuickView>

#include "globals.h"
#include "core/globalManager.h"
#include "terminal/logModule.h"
#include "terminal/terminalPage.h"

// public
TerminalModule::TerminalModule(QWidget *parent)
    : QObject(parent)
      , m_config(g_workspaceConfig["terminalConfig"].toObject()),
      m_manageWindow(new QQuickView()),
      m_terminalModel(new TerminalModel(this)) {
}

TerminalModule::~TerminalModule() {
    delete m_manageWindow;
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] terminal module destructed").arg(timestamp);
}

void TerminalModule::propertySet(const QVariantHash &objects) {
    const auto &terminalMenu = qvariant_cast<QObject *>(objects["terminalModuleTerminalMenu"]);
    terminalMenu->setProperty("terminalModel", QVariant::fromValue(m_terminalModel));

    const auto terminals = m_config["terminals"];
    for (const auto &value: terminals.toArray()) {
        auto terminal = value.toObject();
        const auto name = terminal.take("name").toString();
        const auto session = terminal.contains("session") ? terminal["session"].toObject().toVariantHash() : terminal.toVariantHash();
        auto *item = new QStandardItem(name); // NOLINT
        item->setData(session, Qt::UserRole + 1);
        m_terminalModel->appendRow(item);
    }

    // manage window
    m_manageWindow->setTitle(tr("Manage Terminal"));
    m_manageWindow->setTransientParent(g_mainWindow->windowHandle());

    m_manageWindow->rootContext()->setContextProperty("terminalModule", this);
    m_manageWindow->rootContext()->setContextProperty("global", g_globalManager);
    m_manageWindow->rootContext()->setContextProperty("terminalModel", QVariant::fromValue(m_terminalModel));

    m_manageWindow->setResizeMode(QQuickView::SizeRootObjectToView);
    m_manageWindow->setSource(QUrl("qrc:/qml/terminal/terminalManageWindow.qml"));
}

void TerminalModule::terminalConfigSave() const {
    g_workspaceConfig["terminalConfig"] = m_config;
}

void TerminalModule::terminalManage() const {
    m_manageWindow->resize(1080, 720);
    m_manageWindow->show();
}

void TerminalModule::terminalSave() {
    m_manageWindow->close();
    QJsonArray config{};
    for (int i = 0; i < m_terminalModel->rowCount(); ++i) {
        const auto &name = m_terminalModel->item(i, 0)->text();
        const auto &session = m_terminalModel->item(i, 0)->data(Qt::UserRole + 1).toHash();
        auto terminal = QJsonObject::fromVariantHash(session);
        terminal["name"] = name;
        config.append(terminal);
    }
    m_config["terminals"] = config;
}

int TerminalModule::terminalAdd() const {
    const int row = m_terminalModel->rowCount();
    auto *item = new QStandardItem("new"); // NOLINT
    const auto &session = QVariantHash({
        {"program", ""},
        {"arguments", ""},
        {"workingDirectory", ""},
        {"environment", ""}
    });
    item->setData(session, Qt::UserRole + 1);
    m_terminalModel->appendRow(item);
    return row;
}

void TerminalModule::terminalDelete(const int index) const {
    if (index >= 0 && index < m_terminalModel->rowCount()) m_terminalModel->removeRow(index);
}

void TerminalModule::terminalSwap(const int src, const int dst) const {
    if (src < 0 || src >= m_terminalModel->rowCount() || dst < 0 || dst >= m_terminalModel->rowCount() || src == dst) return;
    const auto row = m_terminalModel->takeRow(src);
    m_terminalModel->insertRow(dst, row);
}

void TerminalModule::terminalOpen(const QString &name, const QVariantHash &session) {
    int index = 0;
    while (m_terminalHash.contains(index)) {
        index++;
    }
    auto *terminalPage = new TerminalPage(QString("%1 %2").arg(name, QString::number(index)), session, m_config);
    g_log->addDockWidgetAsTab(terminalPage);
    terminalPage->propertySet(QVariantHash{
    });
    m_terminalHash.insert(index, terminalPage);
    connect(terminalPage, &TerminalPage::destroyed, this, [this, index] { m_terminalHash.remove(index); });
}

// public
QHash<int, QByteArray> TerminalModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[Qt::UserRole + 1] = "session";
    return roles;
}
