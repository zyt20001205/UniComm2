#ifndef UNICOMM_MCPMODULE_H
#define UNICOMM_MCPMODULE_H

#include <QHash>
#include <QObject>

class QNetworkRequest;
class QStandardItemModel;

class McpModule final : public QObject {
    Q_OBJECT

public:
    explicit McpModule(const QJsonArray &mcpConfig, QObject *parent = nullptr);

    ~McpModule() override = default;

    void initialize();

signals:
    void setModel(QStandardItemModel *mcpModel);

private:
    void toolsList();

    QStandardItemModel *m_mcpModel{};
    QHash<QString, QNetworkRequest> m_requests{};
    int m_id = 0;
};

#endif //UNICOMM_MCPMODULE_H
