#ifndef UNICOMM_CONTEXTMODULE_H
#define UNICOMM_CONTEXTMODULE_H

#include <QJsonArray>
#include <QObject>
#include <QString>

#include "agent/module/sqlModule.h"

class ContextModule final : public QObject {
    Q_OBJECT

public:
    explicit ContextModule(QObject *parent = nullptr);

    [[nodiscard]] QJsonArray contextBuild(int mode, const QList<SqlModule::Message> &history,const QList<SqlModule::Message> &turn) const;

private:
    QString m_system{};
};

#endif //UNICOMM_CONTEXTMODULE_H
