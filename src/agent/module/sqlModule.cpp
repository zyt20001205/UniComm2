#include "agent/module/sqlModule.h"
#include "globals.h"

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QSet>
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
        SELECT id, title, mode, provider, model, created_at, updated_at
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
            .mode = query.value(2).toInt(),
            .provider = query.value(3).toString(),
            .model = query.value(4).toString(),
            .createdAt = query.value(5).toLongLong(),
            .updatedAt = query.value(6).toLongLong()
        });
    }
    return conversations;
}

QPair<SqlModule::Conversation, QList<SqlModule::Message>> SqlModule::conversationGet(const QString &id) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return {};

    QSqlQuery query(database);
    query.prepare(R"(
        SELECT id, title, mode, provider, model, summary, compacted_turn_id, context_tokens, created_at, updated_at
        FROM conversations
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) return {};

    const Conversation conversation{
        .id = query.value(0).toString(),
        .title = query.value(1).toString(),
        .mode = query.value(2).toInt(),
        .provider = query.value(3).toString(),
        .model = query.value(4).toString(),
        .summary = query.value(5).toString(),
        .compactedTurnId = query.value(6).toString(),
        .contextTokens = query.value(7).toLongLong(),
        .createdAt = query.value(8).toLongLong(),
        .updatedAt = query.value(9).toLongLong()
    };

    QSqlQuery messageQuery(database);
    messageQuery.prepare(R"(
        SELECT id, conversation_id, turn_id, sequence, role, content, reasoning_content, tool_call_id, tool_calls, approved, created_at
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
            .approved = messageQuery.value(9).toBool(),
            .createdAt = messageQuery.value(10).toLongLong()
        });
    }
    return {conversation, messages};
}

QList<SqlModule::SearchResult> SqlModule::conversationsSearch(const QString &text, const int limit) const {
    const auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return {};

    QSqlQuery query(database);
    query.prepare(R"(
        SELECT messages.conversation_id, conversations.title, messages.turn_id, messages.created_at, bm25(messages_fts)
        FROM messages_fts
        JOIN messages ON messages.rowid = messages_fts.rowid
        JOIN conversations ON conversations.id = messages.conversation_id
        WHERE messages_fts MATCH :text
          AND messages.turn_id IS NOT NULL
        ORDER BY bm25(messages_fts)
        LIMIT :limit
    )");
    query.bindValue(":text", text);
    query.bindValue(":limit", limit * 8);
    if (!query.exec()) {
        qDebug() << "agent database conversations search failed:" << query.lastError().text();
        return {};
    }

    QSqlQuery messageQuery(database);
    messageQuery.prepare(R"(
        SELECT id, conversation_id, turn_id, sequence, role, content, reasoning_content, tool_call_id, tool_calls, approved, created_at
        FROM messages
        WHERE conversation_id = :conversationId AND turn_id = :turnId
        ORDER BY sequence
    )");

    QSet<QString> turns{};
    QList<SearchResult> results{};
    while (query.next() && results.size() < limit) {
        const auto conversationId = query.value(0).toString();
        const auto turnId = query.value(2).toString();
        const auto key = conversationId + '\n' + turnId;
        if (turns.contains(key)) continue;
        turns.insert(key);

        messageQuery.bindValue(":conversationId", conversationId);
        messageQuery.bindValue(":turnId", turnId);
        if (!messageQuery.exec()) {
            qDebug() << "agent database search result get failed:" << messageQuery.lastError().text();
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
                .approved = messageQuery.value(9).toBool(),
                .createdAt = messageQuery.value(10).toLongLong()
            });
        }
        results.append(SearchResult{
            .conversationId = conversationId,
            .conversationTitle = query.value(1).toString(),
            .turnId = turnId,
            .createdAt = messages.first().createdAt,
            .rank = query.value(4).toDouble(),
            .messages = messages
        });
    }
    return results;
}

void SqlModule::conversationInsert(const Conversation &conversation) const {
    auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        INSERT INTO conversations (id, title, mode, provider, model, created_at, updated_at)
        VALUES (:id, :title, :mode, :provider, :model, :createdAt, :updatedAt)
    )");
    query.bindValue(":id", conversation.id);
    query.bindValue(":title", conversation.title);
    query.bindValue(":mode", conversation.mode);
    query.bindValue(":provider", conversation.provider);
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

void SqlModule::conversationModeSet(const QString &id, const int mode) const {
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

void SqlModule::conversationCompact(const QString &id, const QString &summary, const QString &compactedTurnId) const {
    const auto database = QSqlDatabase::database(m_connectionName, false);
    if (!database.isOpen()) return;

    QSqlQuery query(database);
    query.prepare(R"(
        UPDATE conversations
        SET summary = :summary, compacted_turn_id = :compactedTurnId, context_tokens = 0, updated_at = :updatedAt
        WHERE id = :id
    )");
    query.bindValue(":id", id);
    query.bindValue(":summary", summary);
    query.bindValue(":compactedTurnId", compactedTurnId);
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    if (query.exec()) return;
    qDebug() << "agent database conversation compact failed:" << query.lastError().text();
}

void SqlModule::conversationAppend(const QString &conversationId, const QList<Message> &messages, const qint64 contextTokens) const {
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
            id, conversation_id, turn_id, sequence, role, content, reasoning_content, tool_call_id, tool_calls, approved, created_at
        )
        VALUES (
            :id, :conversationId, :turnId, :sequence, :role, :content, :reasoningContent, :toolCallId, :toolCalls, :approved, :createdAt
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
        query.bindValue(":approved", message.approved);
        query.bindValue(":createdAt", message.createdAt);
        if (query.exec()) continue;

        qDebug() << "agent database message insert failed:" << query.lastError().text();
        database.rollback();
        return;
    }

    query.prepare(R"(
        UPDATE conversations
        SET context_tokens = :contextTokens, updated_at = :updatedAt
        WHERE id = :conversationId
    )");
    query.bindValue(":conversationId", conversationId);
    query.bindValue(":contextTokens", contextTokens);
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
                mode INTEGER,
                provider TEXT,
                model TEXT,
                summary TEXT NOT NULL DEFAULT '',
                compacted_turn_id TEXT NOT NULL DEFAULT '',
                context_tokens INTEGER NOT NULL DEFAULT 0,
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
                approved INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL,
                FOREIGN KEY (conversation_id) REFERENCES conversations(id) ON DELETE CASCADE,
                UNIQUE (conversation_id, sequence)
            )
        )",
        R"(
            CREATE VIRTUAL TABLE IF NOT EXISTS messages_fts USING fts5(
                content,
                reasoning_content,
                content = 'messages',
                content_rowid = 'rowid'
            )
        )",
        R"(
            CREATE TRIGGER IF NOT EXISTS messages_fts_insert AFTER INSERT ON messages BEGIN
                INSERT INTO messages_fts(rowid, content, reasoning_content)
                VALUES (new.rowid, new.content, new.reasoning_content);
            END
        )",
        R"(
            CREATE TRIGGER IF NOT EXISTS messages_fts_delete AFTER DELETE ON messages BEGIN
                INSERT INTO messages_fts(messages_fts, rowid, content, reasoning_content)
                VALUES ('delete', old.rowid, old.content, old.reasoning_content);
            END
        )",
        R"(
            CREATE TRIGGER IF NOT EXISTS messages_fts_update AFTER UPDATE ON messages BEGIN
                INSERT INTO messages_fts(messages_fts, rowid, content, reasoning_content)
                VALUES ('delete', old.rowid, old.content, old.reasoning_content);
                INSERT INTO messages_fts(rowid, content, reasoning_content)
                VALUES (new.rowid, new.content, new.reasoning_content);
            END
        )",
        "CREATE INDEX IF NOT EXISTS conversations_updated_at ON conversations(updated_at DESC)",
        "CREATE INDEX IF NOT EXISTS messages_turn_id ON messages(conversation_id, turn_id, sequence)",
        "PRAGMA user_version = 5"
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
