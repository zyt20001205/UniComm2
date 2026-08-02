#include "agent/module/sqlModule.h"
#include "globals.h"

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
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

QList<SqlModule::Conversation> SqlModule::conversationsGet() const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return {};

    QSqlQuery query(database);
    if (!query.exec(R"(
        SELECT id, title, mode, model, created_at, updated_at
        FROM conversations
        ORDER BY updated_at DESC
    )")) {
        qDebug() << "agent database conversation get failed:" << query.lastError().text();
        return {};
    }

    QList<Conversation> conversations{};
    while (query.next()) {
        conversations.append(Conversation{
            .id = query.value(0).toString(),
            .title = query.value(1).toString(),
            .mode = query.value(2).toString(),
            .model = query.value(3).toString(),
            .createdAt = query.value(4).toLongLong(),
            .updatedAt = query.value(5).toLongLong()
        });
    }
    return conversations;
}

QPair<SqlModule::Conversation, QList<SqlModule::Message>> SqlModule::conversationGet(const QString &id) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return {};

    QSqlQuery query(database);
    query.prepare(R"(
        SELECT id, title, mode, model, created_at, updated_at
        FROM conversations
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) return {};

    const Conversation conversation{
        .id = query.value(0).toString(),
        .title = query.value(1).toString(),
        .mode = query.value(2).toString(),
        .model = query.value(3).toString(),
        .createdAt = query.value(4).toLongLong(),
        .updatedAt = query.value(5).toLongLong()
    };

    QSqlQuery messageQuery(database);
    messageQuery.prepare(R"(
        SELECT id, conversation_id, turn_id, sequence, role, content,
               reasoning_content, tool_call_id, tool_calls, created_at
        FROM messages
        WHERE conversation_id = :conversationId
        ORDER BY sequence
    )");
    messageQuery.bindValue(":conversationId", id);
    if (!messageQuery.exec()) {
        qDebug() << "agent database conversation get failed:" << messageQuery.lastError().text();
        return {};
    }

    QList<Message> messages{};
    while (messageQuery.next()) {
        const auto toolCallsDocument = QJsonDocument::fromJson(messageQuery.value(8).toString().toUtf8());
        messages.append(Message{
            .id = messageQuery.value(0).toString(),
            .conversationId = messageQuery.value(1).toString(),
            .turnId = messageQuery.value(2).toString(),
            .sequence = messageQuery.value(3).toLongLong(),
            .role = messageQuery.value(4).toString(),
            .content = messageQuery.value(5).toString(),
            .reasoningContent = messageQuery.value(6).toString(),
            .toolCallId = messageQuery.value(7).toString(),
            .toolCalls = toolCallsDocument.isArray() ? toolCallsDocument.array() : QJsonArray{},
            .createdAt = messageQuery.value(9).toLongLong()
        });
    }
    return {conversation, messages};
}

void SqlModule::conversationInsert(const Conversation &conversation) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        INSERT INTO conversations (id, title, mode, model, created_at, updated_at)
        VALUES (:id, :title, :mode, :model, :createdAt, :updatedAt)
    )");
    query.bindValue(":id", conversation.id);
    query.bindValue(":title", conversation.title);
    query.bindValue(":mode", conversation.mode);
    query.bindValue(":model", conversation.model);
    query.bindValue(":createdAt", conversation.createdAt);
    query.bindValue(":updatedAt", conversation.updatedAt);
    if (query.exec()) return;
    qDebug() << "agent database conversation insert failed:" << query.lastError().text();
}

void SqlModule::conversationRename(const QString &id, const QString &title) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        UPDATE conversations
        SET title = :title, updated_at = :updatedAt
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    query.bindValue(":title", title);
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    if (query.exec()) return;
    qDebug() << "agent database conversation rename failed:" << query.lastError().text();
}

void SqlModule::conversationDelete(const QString &id) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare("DELETE FROM conversations WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec()) return;
    qDebug() << "agent database conversation delete failed:" << query.lastError().text();
}

void SqlModule::conversationModeSet(const QString &id, const QString &mode) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        UPDATE conversations
        SET mode = :mode, updated_at = :updatedAt
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    query.bindValue(":mode", mode);
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    if (query.exec()) return;
    qDebug() << "agent database conversation mode set failed:" << query.lastError().text();
}

void SqlModule::conversationModelSet(const QString &id, const QString &model) const {
    const auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        UPDATE conversations
        SET model = :model, updated_at = :updatedAt
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    query.bindValue(":model", model);
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    if (query.exec()) return;
    qDebug() << "agent database conversation model set failed:" << query.lastError().text();
}

void SqlModule::conversationAppend(const QString &conversationId, const QList<Message> &messages) const {
    if (messages.isEmpty()) return;

    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen() || !database.transaction()) return;

    QSqlQuery sequenceQuery(database);
    sequenceQuery.prepare(R"(
        SELECT COALESCE(MAX(sequence) + 1, 0)
        FROM messages
        WHERE conversation_id = :conversationId
    )");
    sequenceQuery.bindValue(":conversationId", conversationId);
    if (!sequenceQuery.exec() || !sequenceQuery.next()) {
        qDebug() << "agent database message sequence get failed:" << sequenceQuery.lastError().text();
        database.rollback();
        return;
    }
    auto sequence = sequenceQuery.value(0).toLongLong();
    sequenceQuery.finish();

    QSqlQuery query(database);
    query.prepare(R"(
        INSERT INTO messages (
            id, conversation_id, turn_id, sequence, role, content,
            reasoning_content, tool_call_id, tool_calls, created_at
        )
        VALUES (
            :id, :conversationId, :turnId, :sequence, :role, :content,
            :reasoningContent, :toolCallId, :toolCalls, :createdAt
        )
    )");
    for (const auto &message: messages) {
        query.bindValue(":id", message.id);
        query.bindValue(":conversationId", message.conversationId);
        query.bindValue(":turnId", message.turnId);
        query.bindValue(":sequence", sequence++);
        query.bindValue(":role", message.role);
        query.bindValue(":content", message.content);
        query.bindValue(":reasoningContent", message.reasoningContent);
        query.bindValue(":toolCallId", message.toolCallId);
        query.bindValue(":toolCalls", QString::fromUtf8(QJsonDocument(message.toolCalls).toJson(QJsonDocument::Compact)));
        query.bindValue(":createdAt", message.createdAt);
        if (query.exec()) continue;

        qDebug() << "agent database message insert failed:" << query.lastError().text();
        database.rollback();
        return;
    }

    query.prepare(R"(
        UPDATE conversations
        SET updated_at = :updatedAt
        WHERE id = :conversationId
    )");
    query.bindValue(":conversationId", conversationId);
    query.bindValue(":updatedAt", messages.constLast().createdAt);
    if (!query.exec()) {
        qDebug() << "agent database conversation update failed:" << query.lastError().text();
        database.rollback();
        return;
    }

    if (!database.commit()) database.rollback();
}

void SqlModule::conversationRollback(const QString &conversationId, const QString &turnId) const {
    const auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        DELETE FROM messages
        WHERE conversation_id = :conversationId AND turn_id = :turnId
    )");
    query.bindValue(":conversationId", conversationId);
    query.bindValue(":turnId", turnId);
    if (query.exec()) return;
    qDebug() << "agent database turn delete failed:" << query.lastError().text();
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
                title TEXT NOT NULL,
                mode TEXT,
                model TEXT,
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
                content TEXT,
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
