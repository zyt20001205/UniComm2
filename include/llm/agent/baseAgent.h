#ifndef UNICOMM_BASEAGENT_H
#define UNICOMM_BASEAGENT_H

#include <QNetworkRequest>
#include <QObject>

class QStandardItemModel;

class BaseAgent : public QObject {
    Q_OBJECT

public:
    explicit BaseAgent(QObject *parent = nullptr);

    ~BaseAgent() override = default;

    QNetworkRequest requestGet() { return m_request; }

    virtual void modelGet() = 0;

signals:
    void setModel(QStandardItemModel *agentModel);

protected:
    virtual void keyGet() = 0;

    QNetworkRequest m_request{};
    QByteArray m_key{};
};

#endif //UNICOMM_BASEAGENT_H
