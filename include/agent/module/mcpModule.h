#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QJsonObject>
#include <QObject>

class QNetworkRequest;
class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QJsonObject &mcpConfig, QObject *parent = nullptr);

    ~McpModule() override = default;

    void initialize();

    [[nodiscard]] QString toolsCall(const QString &owner, const QString &name, const QString &arguments);

signals:
    void registerTools(const QString &name, const QJsonArray &tools);

private:
    void toolsList(const QString &name);

    QHash<QString, QNetworkRequest> m_requests{};
    int m_id = 0;
};

#endif //UNICOMM_MCPMODULE_H
