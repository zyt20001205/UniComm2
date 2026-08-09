#ifndef UNICOMM_CONTEXTMODULE_H
#define UNICOMM_CONTEXTMODULE_H

#include "agent/module/sqlModule.h"

class ContextModule final : public QObject {
    Q_OBJECT

public:
    explicit ContextModule(QObject *parent = nullptr);

    [[nodiscard]] QJsonArray contextBuild(const SqlModule::Conversation &conversation, const QList<SqlModule::Message> &history, const QList<SqlModule::Message> &turn) const;

    [[nodiscard]] QPair<QString, QJsonArray> compactBuild(const SqlModule::Conversation &conversation, const QList<SqlModule::Message> &history) const;

private:
    [[nodiscard]] static QJsonObject messageBuild(const SqlModule::Message &message);

    QString m_system{};
};

#endif //UNICOMM_CONTEXTMODULE_H
