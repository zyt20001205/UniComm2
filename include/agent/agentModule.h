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

class AgentModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT
    Q_PROPERTY(int state READ stateGet WRITE stateSet NOTIFY changeState)

public:
    using AgentState = RuntimeModule::AgentState;
    using AgentMode = RuntimeModule::AgentMode;

    explicit AgentModule();

    ~AgentModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void agentConfigSave();

    Q_INVOKABLE void agentManage() const;

    [[nodiscard]] int stateGet() const {
        return m_activeRuntime == nullptr ? m_supervisorRuntime->stateGet() : m_activeRuntime->stateGet();
    }

    void stateSet(int state);

    Q_INVOKABLE void apikeySet(const QString &provider, const QString &apikey) const;

    Q_INVOKABLE void conversationsGet();

    Q_INVOKABLE void conversationGet(const QString &id);

    Q_INVOKABLE void conversationInsert();

    Q_INVOKABLE void conversationRename(const QString &title);

    Q_INVOKABLE void conversationDelete();

    Q_INVOKABLE void conversationModeSet(int mode);

    Q_INVOKABLE void conversationModelSet(const QString &provider, const QString &model);

    Q_INVOKABLE void conversationRollback();

    Q_INVOKABLE void permissionSet(bool status);

    Q_INVOKABLE void userInputSet(const QString &answer) const;

    Q_INVOKABLE void userInputDisable() const;

    [[nodiscard]] RuntimeModule *agentExecute(const QString &role, const QString &task);

signals:
    void changeState();

private:
    void turnCreate(const QString &turnId, qint64 startedAt) const;

    void turnFinish(const QString &turnId, qint64 finishedAt) const;

    void chatCreate(const QString &turnId, const QString &messageId, const QString &role) const;

    void chatAppend(const QString &messageId, const QString &text) const;

    void chatFinish(const QString &messageId) const;

    void modelUpdate(const QString &provider, const QString &model) const;

    QJsonObject m_config{};
    QQuickWidget *m_widget{};
    QQuickView *m_manageWindow{};
    QObject *m_root{};
    QObject *m_toast{};
    QObject *m_modeMenu{};
    QObject *m_conversationComboBox{};
    QObject *m_textArea{};
    QObject *m_permissionLabel{};
    QObject *m_userInputCard{};
    QObject *m_modeButton{};
    QObject *m_modelButton{};

    QString m_conversationId{};
    ConversationModel *m_conversationModel{};
    ContextModule *m_contextModule{};
    ProviderModule *m_providerModule{};
    SqlModule *m_sqlModule{};
    ToolsModule *m_toolsModule{};
    RuntimeModule *m_supervisorRuntime{};
    RuntimeModule *m_activeRuntime{};
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
