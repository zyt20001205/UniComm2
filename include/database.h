#ifndef DATABASE_H
#define DATABASE_H

#include <QDockWidget>
#include <QHeaderView>
#include <QJsonArray>
#include <QTableWidget>
#include "config.h"

class Database final : public QDockWidget {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    ~Database() override = default;

    void databaseConfigSave() const;

    void databaseWrite(const QString &key, const QString &value);

private:
    QJsonArray m_databaseConfig = g_config["databaseConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;

signals:
    void appendLog(const QString &message, const QString &level);
};

#endif //DATABASE_H
