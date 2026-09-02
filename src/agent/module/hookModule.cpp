#include "agent/module/hookModule.h"

#include <QFileInfo>
#include <QJsonArray>

#include "globals.h"
#include "runtime/threadpoolModule.h"
#include "util/uniCast.h"

// public
HookModule::HookModule(const QJsonObject &config, QObject *parent)
    : QObject(parent),
      m_config(config),
      m_hookModel(new HookModel(this)) {
    auto *item = new QStandardItem(tr("Turn finished")); // NOLINT
    item->setData("turnFinish", HookModel::EventRole);
    item->setData(tr("Run scripts after an agent turn finishes."), HookModel::DescriptionRole);
    m_hookModel->appendRow(item);
    hookUpdate("turnFinish");
}

HookModel *HookModule::hookModelGet() const {
    return m_hookModel;
}

const QJsonObject &HookModule::configGet() const {
    return m_config;
}

void HookModule::hookEnabledSet(const QString &event, const bool enabled) {
    auto hook = m_config[event].toObject();
    hook["enabled"] = enabled;
    m_config[event] = hook;
    hookUpdate(event);
}

void HookModule::hookScriptInsert(const QString &event, const QUrl &documentUrl) {
    auto hook = m_config[event].toObject();
    auto scripts = hook["scripts"].toArray();
    if (scripts.contains(documentUrl.toString())) return;
    scripts.append(documentUrl.toString());
    hook["enabled"] = true;
    hook["scripts"] = scripts;
    m_config[event] = hook;
    hookUpdate(event);
}

void HookModule::hookScriptRemove(const QString &event, const QUrl &documentUrl) {
    auto hook = m_config[event].toObject();
    auto scripts = hook["scripts"].toArray();
    for (qsizetype index = 0; index < scripts.size(); ++index) {
        if (scripts.at(index).toString() != documentUrl.toString()) continue;
        scripts.removeAt(index);
        break;
    }
    hook["scripts"] = scripts;
    m_config[event] = hook;
    hookUpdate(event);
}

void HookModule::hookRun(const QString &event) const {
    const auto hook = m_config[event].toObject();
    if (!hook["enabled"].toBool()) return;
    for (const auto &value: hook["scripts"].toArray()) {
        const QUrl documentUrl(value.toString());
        if (!QFileInfo(documentUrl.toLocalFile()).isFile()) {
            qWarning() << "Hook script does not exist:" << documentUrl;
            continue;
        }
        g_threadpool->threadStart(documentUrl, InterpreterMode::Agent);
    }
}

// private
QVariantList HookModule::scriptsGet(const QString &event) const {
    QVariantList scripts{};
    for (const auto &value: m_config[event].toObject()["scripts"].toArray()) {
        const QUrl documentUrl(value.toString());
        scripts.append(QVariantMap{
            {"documentUrl", documentUrl},
            {"fileName", documentUrl.fileName()},
            {"decoration", uni_cast<QFileIcon>(documentUrl).value}
        });
    }
    return scripts;
}

void HookModule::hookUpdate(const QString &event) const {
    const auto index = m_hookModel->match(m_hookModel->index(0, 0), HookModel::EventRole, event).constFirst();
    auto *item = m_hookModel->itemFromIndex(index);
    const auto hook = m_config[event].toObject();
    item->setData(hook["enabled"].toBool(), HookModel::EnabledRole);
    item->setData(scriptsGet(event), HookModel::ScriptsRole);
}

// public
HookModel::HookModel(QObject *parent)
    : QStandardItemModel(parent) {
}

QHash<int, QByteArray> HookModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[EventRole] = "event";
    roles[DescriptionRole] = "description";
    roles[EnabledRole] = "enabled";
    roles[ScriptsRole] = "scripts";
    return roles;
}
