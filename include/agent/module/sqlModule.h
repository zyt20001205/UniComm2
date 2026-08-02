#ifndef UNICOMM_SQLMODULE_H
#define UNICOMM_SQLMODULE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

class SqlModule final : public QObject {
    Q_OBJECT

public:
    struct Conversation {
        QString id{};
        QString legacyTopic{};
        QString title{};
        QString mode{};
        QString model{};
        qint64 createdAt{};
        qint64 updatedAt{};
    };

    struct Message {
        QString id{};
        QString conversationId{};
        QString turnId{};
        qint64 sequence{};
        QString role{};
        QString content{};
        QString reasoningContent{};
        QString toolCallId{};
        QJsonArray toolCalls{};
        qint64 createdAt{};
    };

    explicit SqlModule(const QJsonObject &sqlConfig, QObject *parent = nullptr);

    ~SqlModule() override;

    [[nodiscard]] QList<Conversation> conversationGet() const;

    void conversationInsert(const Conversation &conversation) const;

    void conversationRename(const QString &id, const QString &title) const;

    void conversationDelete(const QString &id) const;

    void conversationModeSet(const QString &id, const QString &mode) const;

    void conversationModelSet(const QString &id, const QString &model) const;

    [[nodiscard]] QList<Message> messageGet(const QString &conversationId) const;

    void messageInsert(const Message &message) const;

    void messageDeleteFrom(const QString &conversationId, qint64 sequence) const;

private:
    [[nodiscard]] bool initialize() const;

    QJsonObject m_config{};
    QString m_connectionName{};
};

#endif //UNICOMM_SQLMODULE_H
