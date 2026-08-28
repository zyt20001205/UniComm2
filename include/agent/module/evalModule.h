#ifndef UNICOMM_EVALMODULE_H
#define UNICOMM_EVALMODULE_H

#include <QList>
#include <QObject>

#include "agent/module/sqlModule.h"

class EvalModule final : public QObject {
    Q_OBJECT

public:
    struct Trace {
        QString turnId{};
        QString conversationId{};
        QString provider{};
        QString model{};
        int strategy{};
        int status{};
        QString error{};
        SqlModule::Timing timing{};
        qint64 toolDuration{};
        int modelCalls{};
        int toolCalls{};
        SqlModule::Usage usage{};
    };

    explicit EvalModule(SqlModule *sqlModule, QObject *parent = nullptr);

    [[nodiscard]] Trace traceGet(const QString &turnId) const;

    [[nodiscard]] QList<Trace> tracesGet(const QString &conversationId) const;

private:
    [[nodiscard]] static Trace traceBuild(const QList<SqlModule::Message> &messages);

    SqlModule *m_sqlModule{};
};

#endif //UNICOMM_EVALMODULE_H
