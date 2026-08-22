#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QByteArray>
#include <QFuture>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QStandardItemModel>
#include <QUrl>

class QStandardItem;
class McpModel;

class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QJsonObject &mcpConfig, QObject *parent = nullptr);

    ~McpModule() override = default;

    void initialize();

    [[nodiscard]] QString serverInsert(const QUrl &serverUrl);

    void serverRemove(const QUrl &serverUrl);

    void enabledSet(const QUrl &serverUrl, bool enabled);

    void toolsGet();

    [[nodiscard]] QFuture<QString> toolExecute(const QString &name, const QString &arguments);

    [[nodiscard]] McpModel *mcpModelGet() const;

signals:
    void registerTools(const QJsonArray &tools);

private:
    struct Response {
        QByteArray buffer{};
        QJsonObject object{};
        bool eventStream{};
    };

    struct Tool {
        QUrl serverUrl{};
        QString name{};
    };

    void initialize(const QUrl &serverUrl);

    void toolsGet(const QUrl &serverUrl, const QString &cursor, QJsonArray tools);

    [[nodiscard]] QFuture<QJsonObject> request(const QUrl &serverUrl, const QString &method, QJsonObject params, const QString &name = {});

    [[nodiscard]] static QByteArray headerValueGet(const QString &value);

    static void responseRead(Response &response, int id, bool finished);

    QHash<QUrl, QStandardItem *> m_servers{};
    QHash<QString, Tool> m_tools{};
    McpModel *m_mcpModel{};
    int m_id = 0;
};

class McpModel final : public QStandardItemModel {
    Q_OBJECT

public:
    using QStandardItemModel::QStandardItemModel;

    enum Role {
        UrlRole = Qt::UserRole + 1,
        EnabledRole,
        VersionRole,
        DescriptionRole,
        WebsiteUrlRole,
        InstructionsRole,
        SupportedVersionsRole,
        CapabilitiesRole,
        CacheScopeRole,
        TtlMsRole
    };

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

#endif //UNICOMM_MCPMODULE_H
