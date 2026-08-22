#ifndef UNICOMM_AGENTMODULE_H
#define UNICOMM_AGENTMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QHash>
#include <QStandardItemModel>

#include "agent/runtime/runtimeModule.h"

class QQuickView;
class QQuickWidget;

class ConversationModel;
class ContextModule;
class ProviderModule;
class SqlModule;
class ToolsModule;
class ToastModule;

class AgentModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT
    Q_PROPERTY(int state READ stateGet NOTIFY changeState)

public:
    using AgentState = RuntimeModule::AgentState;
    using AgentMode = RuntimeModule::AgentMode;

    struct AgentStrategy {
        enum {
            Solo,
            Team
        };
    };

    explicit AgentModule();

    ~AgentModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void agentConfigSave();

    Q_INVOKABLE void agentManage() const;

    [[nodiscard]] RuntimeServices runtimeServicesGet() const;

    [[nodiscard]] int stateGet() const;

    Q_INVOKABLE void apikeySet(const QString &provider, const QString &apikey) const;

    Q_INVOKABLE void conversationsGet();

    Q_INVOKABLE void conversationGet(const QString &id);

    Q_INVOKABLE void conversationInsert();

    Q_INVOKABLE void conversationRename(const QString &title);

    Q_INVOKABLE void conversationDelete();

    Q_INVOKABLE void conversationStrategySet(int strategy);

    Q_INVOKABLE void conversationModeSet(int mode);

    Q_INVOKABLE void conversationModelSet(const QString &provider, const QString &model);

    Q_INVOKABLE void conversationRollback();

    Q_INVOKABLE void abort() const;

    Q_INVOKABLE void pre();

    Q_INVOKABLE void compact() const;

    Q_INVOKABLE void permission(const QString &runtimeId, bool status) const;

    Q_INVOKABLE void userInput(const QString &runtimeId, const QString &answer) const;

    void permissionRequest(const QString &runtimeId, const QString &message) const;

    void userInputRequest(const QString &runtimeId, const QVariantMap &request) const;

    void planUpdate(const QString &runtimeId, const QJsonObject &plan) const;

    [[nodiscard]] RuntimeModule *subagentDispatch(const QString &role, const QString &task);

    void subagentCreate(const QString &turnId, const QString &runtimeId, const QString &role, const QString &message) const;

    void subagentUpdate(const QString &runtimeId, const QString &message) const;

signals:
    void changeState();

private:
    void primaryRuntimeConnect(RuntimeModule *runtime);

    void modelUpdate(const QString &provider, const QString &model) const;

    void turnCreate(const QString &turnId, qint64 startedAt) const;

    void turnFinish(const QString &turnId, qint64 finishedAt) const;

    void chatCreate(const QString &turnId, const QString &messageId, const QString &role) const;

    void chatAppend(const QString &messageId, const QString &text) const;

    void chatFinish(const QString &messageId) const;

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QQuickView *m_manageWindow{};
    QObject *m_root{};
    ToastModule *m_toast{};
    QObject *m_modeMenu{};
    QObject *m_conversationComboBox{};
    QObject *m_textArea{};
    QObject *m_strategyButton{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};

    QString m_conversationId{};
    ConversationModel *m_conversationModel{};
    ContextModule *m_contextModule{};
    ProviderModule *m_providerModule{};
    SqlModule *m_sqlModule{};
    ToolsModule *m_toolsModule{};
    QString m_general{};
    QString m_primary{};
    QString m_supervisor{};
    QHash<QString, RuntimeModule *> m_runtimes{};
};

class ConversationModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        IdRole = Qt::UserRole + 1
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_AGENTMODULE_H
