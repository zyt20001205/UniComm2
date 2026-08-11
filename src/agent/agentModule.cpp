#include "agent/agentModule.h"

#include <QDateTime>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWidget>
#include <QUuid>

#include "globals.h"
#include "agent/module/contextModule.h"
#include "agent/module/sqlModule.h"
#include "agent/module/toolsModule.h"
#include "agent/provider/baseProvider.h"
#include "agent/provider/providerModule.h"
#include "agent/role/hardwareAgent.h"
#include "agent/role/supervisorAgent.h"
#include "core/globalManager.h"
#include "document/documentModule.h"

// public
AgentModule::AgentModule()
    : DockWidget("Agent"),
      m_config(g_workspaceConfig["agentConfig"].toObject()),
      m_widget(new QQuickWidget()),
      m_manageWindow(new QQuickView()),
      m_conversationId(m_config["id"].toString()),
      m_conversationModel(new ConversationModel(this)),
      m_contextModule(new ContextModule(this)),
      m_providerModule(new ProviderModule(m_config["providers"].toArray(), this)),
      m_sqlModule(new SqlModule(m_config["sql"].toObject(), this)),
      m_toolsModule(new ToolsModule(m_sqlModule, this)) {
    auto *runtime = new RuntimeModule(new SupervisorAgent(), runtimeServicesGet(), this); // NOLINT
    m_supervisor = runtime->idGet();
    m_runtimes.insert(m_supervisor, runtime);
    setWidget(m_widget);

    conversationsGet();
}

AgentModule::~AgentModule() {
    delete m_manageWindow;
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void AgentModule::propertySet(const QVariantHash &objects) {
    m_toast = qvariant_cast<QObject *>(objects["mainWindowToast"]);
    m_modeMenu = qvariant_cast<QObject *>(objects["agentModuleModeMenu"]);

    m_manageWindow->setTitle(tr("Agent Settings"));
    m_manageWindow->setTransientParent(g_mainWindow->windowHandle());
    m_manageWindow->rootContext()->setContextProperty("agentModule", this);
    m_manageWindow->rootContext()->setContextProperty("global", g_globalManager);
    m_manageWindow->setResizeMode(QQuickView::SizeRootObjectToView);
    m_manageWindow->setSource(QUrl("qrc:/qml/agent/agentManageWindow.qml"));

    m_widget->rootContext()->setContextProperty("agentModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("mainLinkMenu", objects["mainWindowLinkMenu"]);
    m_widget->rootContext()->setContextProperty("mainToolTip", objects["mainWindowToolTip"]);
    m_widget->rootContext()->setContextProperty("documentModule", objects["documentModule"]);
    m_widget->rootContext()->setContextProperty("renameDialog", objects["agentModuleRenameDialog"]);
    m_widget->rootContext()->setContextProperty("conversationModel", m_conversationModel);
    m_widget->rootContext()->setContextProperty("modeMenu", m_modeMenu);
    m_widget->rootContext()->setContextProperty("modelMenu", objects["agentModuleModelMenu"]);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/agent/agentModule.qml"));
    m_root = m_widget->rootObject();

    auto *supervisor = m_runtimes.value(m_supervisor);
    connect(supervisor, &RuntimeModule::changeState, this, &AgentModule::changeState);
    connect(supervisor, &RuntimeModule::showError, this, [this](const QString &message) {
        QMetaObject::invokeMethod(m_toast, "show", Q_ARG(int, 0), Q_ARG(QString, tr("Agent")), Q_ARG(QString, message), Q_ARG(int, 5000));
    });
    connect(supervisor, &RuntimeModule::createTurn, this, [this](const QString &turnId, const qint64 startedAt) {
        turnCreate(turnId, startedAt);
        QMetaObject::invokeMethod(m_textArea, "clear");
    });
    connect(supervisor, &RuntimeModule::finishTurn, this, &AgentModule::turnFinish);
    connect(supervisor, &RuntimeModule::createChat, this, &AgentModule::chatCreate);
    connect(supervisor, &RuntimeModule::appendChat, this, &AgentModule::chatAppend);
    connect(supervisor, &RuntimeModule::appendChatReasoning, this, [this](const QString &messageId, const QString &text) {
        QMetaObject::invokeMethod(m_root, "chatReasoningAppend", Q_ARG(QString, messageId), Q_ARG(QString, text));
    });
    connect(supervisor, &RuntimeModule::finishChat, this, &AgentModule::chatFinish);
    connect(supervisor, &RuntimeModule::requestPermission, this, [this](const QString &message) {
        m_permissionLabel->setProperty("message", message);
    });
    connect(supervisor, &RuntimeModule::requestUserInput, this, [this](const QVariantMap &request) {
        m_userInputCard->setProperty("request", request);
    });
    connect(supervisor, &RuntimeModule::updatePlan, this, [this](const QJsonObject &plan) {
        QMetaObject::invokeMethod(m_root, "planUpdate", Q_ARG(QVariant, plan.toVariantMap()));
    });
    connect(supervisor, &RuntimeModule::updateUsage, this, [this](const qint64 totalTokens) {
        QMetaObject::invokeMethod(m_root, "usageUpdate", Q_ARG(double, totalTokens));
    });
    connect(supervisor, &RuntimeModule::finishCompact, this, [this] {
        QMetaObject::invokeMethod(m_root, "compactFinish");
    });
    m_toolsModule->initialize();

    m_providerModule->propertySet(QVariantHash{
        {"agentModuleModelMenu", objects["agentModuleModelMenu"]}
    });
    connect(m_providerModule, &ProviderModule::modelsChanged, this, [this] {
        if (m_conversationId.isEmpty()) return;
        const auto conversation = m_sqlModule->conversationGet(m_conversationId).first;
        modelUpdate(conversation.provider, conversation.model);
    });
    m_providerModule->initialize();

    if (!m_conversationId.isEmpty()) {
        m_conversationComboBox->setProperty("currentValue", m_conversationId);
        conversationGet(m_conversationComboBox->property("currentValue").toString());
    }
}

void AgentModule::propertyGet(const QVariantMap &objects) {
    m_conversationComboBox = qvariant_cast<QObject *>(objects["conversationComboBox"]);
    m_textArea = qvariant_cast<QObject *>(objects["textArea"]);
    m_permissionLabel = qvariant_cast<QObject *>(objects["permissionLabel"]);
    m_userInputCard = qvariant_cast<QObject *>(objects["userInputCard"]);
    m_modeButton = qvariant_cast<QObject *>(objects["modeButton"]);
    m_modelButton = qvariant_cast<QObject *>(objects["modelButton"]);
}

void AgentModule::agentConfigSave() {
    m_config["id"] = m_conversationId;
    g_workspaceConfig["agentConfig"] = m_config;
}

void AgentModule::agentManage() const {
    m_manageWindow->resize(1080, 720);
    m_manageWindow->show();
}

RuntimeServices AgentModule::runtimeServicesGet() const {
    return {m_contextModule, m_providerModule, m_sqlModule, m_toolsModule};
}

int AgentModule::stateGet() const {
    return m_runtimes.value(m_active.isEmpty() ? m_supervisor : m_active)->stateGet();
}

void AgentModule::stateSet(const int state) {
    switch (state) {
        case AgentState::Pre: {
            if (m_conversationComboBox->property("currentValue").toString().isEmpty()) conversationInsert();
            m_runtimes.value(m_supervisor)->start(m_conversationId, m_textArea->property("text").toString());
        }
        break;
        case AgentState::Compact: m_runtimes.value(m_supervisor)->compact(m_conversationId);
        break;
        case AgentState::Abort: {
            m_runtimes.value(m_supervisor)->abort();
            if (!m_active.isEmpty()) {
                auto *runtime = m_runtimes.value(m_active);
                m_active.clear();
                runtime->abort();
            }
        }
        break;
        default: break;
    }
}

void AgentModule::apikeySet(const QString &provider, const QString &apikey) const {
    m_providerModule->apikeySet(provider, apikey);
}

void AgentModule::conversationsGet() {
    const auto conversationId = m_conversationId;
    const auto conversations = m_sqlModule->conversationsGet();
    auto currentIndex = -1;
    SqlModule::Conversation currentConversation{};

    m_conversationModel->clear();
    for (const auto &conversation: conversations) {
        auto *item = new QStandardItem(conversation.title); // NOLINT
        item->setData(conversation.id, ConversationModel::IdRole);
        m_conversationModel->appendRow(item);

        if (!currentConversation.id.isEmpty() && conversation.id != conversationId) continue;
        currentIndex = m_conversationModel->rowCount() - 1;
        currentConversation = conversation;
    }

    m_conversationId = currentConversation.id;

    if (m_conversationComboBox == nullptr || m_modeButton == nullptr || m_modelButton == nullptr) return;
    m_conversationComboBox->setProperty("currentIndex", currentIndex);
    m_modeButton->setProperty("mode", currentConversation.id.isEmpty() ? AgentMode::Chat : currentConversation.mode);
    m_modeMenu->setProperty("selectedIndex", currentConversation.id.isEmpty() ? AgentMode::Chat : currentConversation.mode);
    modelUpdate(currentConversation.provider, currentConversation.model);
}

void AgentModule::conversationGet(const QString &id) {
    if (m_modeButton == nullptr || m_modelButton == nullptr) return;
    QMetaObject::invokeMethod(m_root, "chatClear");
    const auto [conversation, messages] = m_sqlModule->conversationGet(id);
    if (conversation.id.isEmpty()) {
        m_conversationId.clear();
        m_modeButton->setProperty("mode", AgentMode::Chat);
        m_modeMenu->setProperty("selectedIndex", AgentMode::Chat);
        modelUpdate({}, {});
        return;
    }

    m_conversationId = id;
    QString turnId{};
    qint64 finishedAt{};
    QHash<QString, QPair<QString, QString>> toolCalls{};
    for (const auto &message: messages) {
        const auto &role = message.role;
        if (message.turnId != turnId) {
            if (!turnId.isEmpty()) turnFinish(turnId, finishedAt);
            turnId = message.turnId;
            turnCreate(turnId, message.createdAt);
        }
        finishedAt = message.createdAt;
        for (const auto &value: message.toolCalls) {
            const auto object = value.toObject();
            const auto function = object.value("function").toObject();
            const auto id = object.value("id").toString();
            toolCalls[id] = {function.value("name").toString(), function.value("arguments").toString()};
        }
        if (role == "tool") {
            const auto toolCall = toolCalls.value(message.toolCallId);
            chatCreate(turnId, message.id, role);
            chatAppend(message.id, m_toolsModule->toolTextGet(toolCall.first, toolCall.second));
            chatAppend(message.id, message.approved ? " ✓" : " ✗");
            chatFinish(message.id);
            continue;
        }
        const auto &content = message.content;
        if (!content.isEmpty()) {
            chatCreate(turnId, message.id, role);
            chatAppend(message.id, content);
            chatFinish(message.id);
        }
    }
    if (!turnId.isEmpty()) turnFinish(turnId, finishedAt);
    m_modeButton->setProperty("mode", conversation.mode);
    m_modeMenu->setProperty("selectedIndex", conversation.mode);
    modelUpdate(conversation.provider, conversation.model);
    QMetaObject::invokeMethod(m_root, "usageUpdate", Q_ARG(double, conversation.contextTokens));
    QMetaObject::invokeMethod(m_root, "followToTail", Qt::QueuedConnection);
}

void AgentModule::conversationInsert() {
    const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const auto timestamp = QDateTime::currentMSecsSinceEpoch();
    m_sqlModule->conversationInsert(SqlModule::Conversation{
        .id = id,
        .title = id,
        .createdAt = timestamp,
        .updatedAt = timestamp
    });
    m_conversationId = id;
    conversationsGet();
}

void AgentModule::conversationRename(const QString &title) {
    if (title.isEmpty()) return;
    m_sqlModule->conversationRename(m_conversationId, title);
    conversationsGet();
}

void AgentModule::conversationDelete() {
    m_sqlModule->conversationDelete(m_conversationId);
    conversationsGet();
}

void AgentModule::conversationModeSet(const int mode) {
    if (m_conversationId.isEmpty()) conversationInsert();
    m_sqlModule->conversationModeSet(m_conversationId, mode);
    conversationsGet();
}

void AgentModule::conversationModelSet(const QString &provider, const QString &model) {
    if (m_conversationId.isEmpty()) conversationInsert();
    m_sqlModule->conversationModelSet(m_conversationId, provider, model);
    modelUpdate(provider, model);
}

void AgentModule::conversationRollback() {
    const auto messages = m_sqlModule->conversationGet(m_conversationId).second;
    if (messages.isEmpty()) return;
    for (auto i = messages.size() - 1; i >= 0; --i) {
        const auto &message = messages.at(i);
        if (message.role == "user") {
            m_textArea->setProperty("text", message.content);
            m_sqlModule->conversationRollback(m_conversationId, message.turnId);
            break;
        }
    }
    conversationGet(m_conversationId);
}

void AgentModule::permissionSet(const bool status) const {
    m_runtimes.value(m_active.isEmpty() ? m_supervisor : m_active)->permissionSet(status);
}

void AgentModule::userInputSet(const QString &answer) const {
    m_runtimes.value(m_active.isEmpty() ? m_supervisor : m_active)->userInputSet(answer);
}

void AgentModule::userInputDisable() const {
    m_runtimes.value(m_active.isEmpty() ? m_supervisor : m_active)->userInputDisable();
}

RuntimeModule *AgentModule::agentExecute(const QString &role, const QString &task) {
    BaseAgent *agent{};
    if (role == "hardware") agent = new HardwareAgent();
    if (agent == nullptr) return nullptr;

    const auto conversation = m_sqlModule->conversationGet(m_conversationId).first;
    auto *worker = new RuntimeModule(agent, runtimeServicesGet(), this); // NOLINT
    m_active = worker->idGet();
    m_runtimes.insert(m_active, worker);
    connect(worker, &RuntimeModule::changeState, this, &AgentModule::changeState);
    connect(worker, &RuntimeModule::requestPermission, this, [this](const QString &message) {
        m_permissionLabel->setProperty("message", message);
    });
    connect(worker, &RuntimeModule::requestUserInput, this, [this](const QVariantMap &request) {
        m_userInputCard->setProperty("request", request);
    });
    connect(worker, &RuntimeModule::finishRun, worker, [this, worker] {
        if (m_active == worker->idGet()) m_active.clear();
        m_runtimes.remove(worker->idGet());
        emit changeState();
        worker->deleteLater();
    });
    worker->startTask(conversation.provider, conversation.model, conversation.mode, task);
    return worker;
}

// private
void AgentModule::turnCreate(const QString &turnId, const qint64 startedAt) const {
    QMetaObject::invokeMethod(m_root, "turnCreate", Q_ARG(QString, turnId), Q_ARG(double, startedAt));
}

void AgentModule::turnFinish(const QString &turnId, const qint64 finishedAt) const {
    QMetaObject::invokeMethod(m_root, "turnFinish", Q_ARG(QString, turnId), Q_ARG(double, finishedAt));
}

void AgentModule::chatCreate(const QString &turnId, const QString &messageId, const QString &role) const {
    QMetaObject::invokeMethod(m_root, "chatCreate", Q_ARG(QString, turnId), Q_ARG(QString, messageId), Q_ARG(QString, role));
}

void AgentModule::chatAppend(const QString &messageId, const QString &text) const {
    QMetaObject::invokeMethod(m_root, "chatAppend", Q_ARG(QString, messageId), Q_ARG(QString, text));
}

void AgentModule::chatFinish(const QString &messageId) const {
    QMetaObject::invokeMethod(m_root, "chatFinish", Q_ARG(QString, messageId));
}

void AgentModule::modelUpdate(const QString &provider, const QString &model) const {
    if (provider.isEmpty() || model.isEmpty()) {
        m_modelButton->setProperty("text", "");
        QMetaObject::invokeMethod(m_root, "modelUpdate", Q_ARG(double, 0));
        return;
    }
    const auto modelInfo = m_providerModule->providerGet(provider)->modelGet(model);
    m_modelButton->setProperty("text", modelInfo.name);
    QMetaObject::invokeMethod(m_root, "modelUpdate", Q_ARG(double, modelInfo.contextWindow));
}

// public
QHash<int, QByteArray> ConversationModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[IdRole] = "id";
    return roles;
}
