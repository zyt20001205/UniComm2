#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QFuture>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QUrl>

class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QJsonObject &mcpConfig, QObject *parent = nullptr);

    ~McpModule() override = default;

    void toolsGet();

    [[nodiscard]] QFuture<QString> toolExecute(const QString &name, const QString &arguments);

signals:
    void registerTools(const QJsonArray &tools);

private:
    struct Response {
        QByteArray buffer{};
        QJsonObject object{};
        bool eventStream{};
    };

    struct Tool {
        QString serverId{};
        QString name{};
    };

    void toolsGet(const QString &serverId, const QString &cursor, QJsonArray tools);

    [[nodiscard]] QFuture<QJsonObject> request(const QString &serverId, const QString &method, QJsonObject params, const QString &name = {});

    [[nodiscard]] static QByteArray headerValueGet(const QString &value);

    static void responseRead(Response &response, int id, bool finished);

    QHash<QString, QUrl> m_servers{};
    QHash<QString, Tool> m_tools{};
    int m_id = 0;
};

#endif //UNICOMM_MCPMODULE_H
