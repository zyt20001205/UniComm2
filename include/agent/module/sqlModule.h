#ifndef UNICOMM_SQLMODULE_H
#define UNICOMM_SQLMODULE_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class SqlModule final : public QObject {
    Q_OBJECT

public:
    explicit SqlModule(const QJsonObject &sqlConfig, QObject *parent = nullptr);

    ~SqlModule() override;

private:
    [[nodiscard]] bool initialize() const;

    QJsonObject m_config{};
    QString m_connectionName{};
};

#endif //UNICOMM_SQLMODULE_H
