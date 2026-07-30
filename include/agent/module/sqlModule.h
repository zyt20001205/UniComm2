#ifndef UNICOMM_SQLMODULE_H
#define UNICOMM_SQLMODULE_H

#include <QJsonObject>
#include <QObject>

class SqlModule final : public QObject {
    Q_OBJECT

public:
    explicit SqlModule(const QJsonObject &sqlConfig, QObject *parent = nullptr);

    ~SqlModule() override = default;

    [[nodiscard]] bool probe() const;
};

#endif //UNICOMM_SQLMODULE_H
