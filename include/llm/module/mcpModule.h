#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QJsonObject>
#include <QObject>

class QNetworkRequest;
class QStandardItemModel;

class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QJsonObject &mcpConfig, QObject *parent = nullptr);

    ~McpModule() override = default;

    void initialize();

    [[nodiscard]] QString toolsCall(const QString &owner, const QString &name, const QString &arguments);

signals:
    void setModel(QStandardItemModel *mcpModel);

    void registerTools(const QString &name, const QJsonArray &tools);

private:
    void toolsList(const QString &name);

    QJsonObject m_mcpConfig{};
    QStandardItemModel *m_mcpModel{};
    QHash<QString, QNetworkRequest> m_requests{};
    int m_id = 0;
};

#endif //UNICOMM_MCPMODULE_H
