#include "agent/module/sqlModule.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

// public
SqlModule::SqlModule(const QJsonObject &sqlConfig, QObject *parent)
    : QObject(parent) {
    Q_UNUSED(sqlConfig)
}

bool SqlModule::probe() const {
    const auto connectionName = "sql_probe_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool success{};
    {
        auto database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        database.setDatabaseName(":memory:");
        if (!database.open()) {
            qWarning() << "SQLite probe open failed:" << database.lastError().text();
        } else {
            QSqlQuery query(database);
            success = query.exec("CREATE TABLE probe (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
                      && query.exec("INSERT INTO probe VALUES ('hello', 'sqlite')")
                      && query.exec("SELECT value FROM probe WHERE key = 'hello'")
                      && query.next()
                      && query.value(0).toString() == "sqlite";
            if (!success) qWarning() << "SQLite probe query failed:" << query.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}
