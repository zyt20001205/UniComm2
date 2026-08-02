#include "agent/module/sqlModule.h"
#include "globals.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>

// public
SqlModule::SqlModule(const QJsonObject &sqlConfig, QObject *parent)
    : QObject(parent),
      m_config(sqlConfig),
      m_connectionName("agent_" + QUuid::createUuid().toString(QUuid::WithoutBraces)) {
    const auto metadataPath = QDir(g_workspaceUrl.toLocalFile()).filePath(".unicomm");
    auto database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    database.setDatabaseName(QDir(metadataPath).filePath("agent.db"));
    if (!database.open()) return;
    if (!initialize()) {
        database.close();
        return;
    }
}

SqlModule::~SqlModule() {
    {
        auto database = QSqlDatabase::database(m_connectionName, false);
        if (database.isValid()) database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

// private
bool SqlModule::initialize() const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return false;

    const QStringList pragmas{
        "PRAGMA foreign_keys = ON",
        "PRAGMA journal_mode = WAL",
        "PRAGMA synchronous = NORMAL",
        "PRAGMA busy_timeout = 5000"
    };
    for (const auto &statement: pragmas) {
        QSqlQuery query(database);
        if (query.exec(statement)) continue;
        qDebug() << "agent database pragma failed:" << query.lastError().text();
        return false;
    }

    if (!database.transaction()) return false;
    const QStringList schema{
        R"(
            CREATE TABLE IF NOT EXISTS agent_meta (
                key TEXT PRIMARY KEY,
                value TEXT NOT NULL
            )
        )",
        R"(
            CREATE TABLE IF NOT EXISTS conversations (
                id TEXT PRIMARY KEY,
                legacy_topic TEXT UNIQUE,
                title TEXT NOT NULL,
                mode TEXT NOT NULL DEFAULT '',
                model TEXT NOT NULL DEFAULT '',
                created_at INTEGER NOT NULL,
                updated_at INTEGER NOT NULL
            )
        )",
        R"(
            CREATE TABLE IF NOT EXISTS messages (
                id TEXT PRIMARY KEY,
                conversation_id TEXT NOT NULL,
                turn_id TEXT,
                sequence INTEGER NOT NULL,
                role TEXT NOT NULL,
                content TEXT NOT NULL DEFAULT '',
                reasoning_content TEXT,
                tool_call_id TEXT,
                tool_calls TEXT,
                created_at INTEGER NOT NULL,
                FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
                UNIQUE (conversation_id, sequence)
            )
        )",
        "CREATE INDEX IF NOT EXISTS conversations_updated_at ON conversations(updated_at DESC)",
        "CREATE INDEX IF NOT EXISTS messages_turn_id ON messages(conversation_id, turn_id, sequence)",
        "PRAGMA user_version = 1"
    };
    for (const auto &statement: schema) {
        QSqlQuery query(database);
        if (query.exec(statement)) continue;
        qDebug() << "agent database schema failed:" << query.lastError().text();
        database.rollback();
        return false;
    }

    if (database.commit()) return true;
    qDebug() << "agent database commit failed:" << database.lastError().text();
    database.rollback();
    return false;
}
