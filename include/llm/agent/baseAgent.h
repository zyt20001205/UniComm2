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

    virtual void apikeyGet() = 0;

    virtual void apikeySet(const QString &apikey) = 0;

    virtual void modelGet() = 0;

signals:
    void setKey(const QString &apikey);

    void setModel(QStandardItemModel *agentModel);

protected:
    QNetworkRequest m_request{};
    QString m_service{};
    QString m_key{};
    QString m_apikey{};
};

#endif //UNICOMM_BASEAGENT_H
