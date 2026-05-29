#ifndef UNICOMM_BASEPROVIDER_H
#define UNICOMM_BASEPROVIDER_H

#include <QNetworkRequest>
#include <QObject>

class QStandardItemModel;

class BaseProvider : public QObject {
    Q_OBJECT

public:
    explicit BaseProvider(QObject *parent = nullptr);

    ~BaseProvider() override = default;

    QNetworkRequest requestGet() { return m_request; }

    virtual void apikeyGet() = 0;

    virtual void apikeySet(const QString &apikey) = 0;

    virtual void modelGet() = 0;

signals:
    void setApikey(const QString &apikey);

    void setModel(QStandardItemModel *agentModel);

protected:
    QNetworkRequest m_request{};
    QString m_service{};
    QString m_key{};
    QString m_apikey{};
};

#endif //UNICOMM_BASEPROVIDER_H
