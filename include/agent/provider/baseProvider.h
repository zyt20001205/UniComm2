#ifndef UNICOMM_BASEPROVIDER_H
#define UNICOMM_BASEPROVIDER_H

#include <QList>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

class QStandardItemModel;

class BaseProvider : public QObject {
    Q_OBJECT

public:
    struct Model {
        QString id{};
        QString name{};
        qint64 contextWindow{};
        qint64 maxOutputTokens{};
    };

    explicit BaseProvider(QObject *parent = nullptr);

    ~BaseProvider() override = default;

    QNetworkRequest requestGet() { return m_request; }

    void catalogSet(const QJsonObject &catalog) {
        m_catalog = catalog;
    }

    virtual void apikeyGet() = 0;

    virtual void apikeySet(const QString &apikey) = 0;

    virtual void modelsGet() = 0;

    [[nodiscard]] virtual Model modelGet(const QString &id) const = 0;

signals:
    void setApikey(const QString &apikey);

    void setModel(QStandardItemModel *providerModel);

protected:
    QNetworkRequest m_request{};
    QString m_service{};
    QString m_key{};
    QString m_apikey{};
    QJsonObject m_catalog{};
    QList<Model> m_models{};
};

#endif //UNICOMM_BASEPROVIDER_H
