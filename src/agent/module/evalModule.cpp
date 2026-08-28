#include "agent/module/evalModule.h"

#include "agent/agentModule.h"

EvalModule::EvalModule(SqlModule *sqlModule, QObject *parent) : QObject(parent), m_sqlModule(sqlModule) {
}

EvalModule::Trace EvalModule::traceGet(const QString &turnId) const {
    return traceBuild(m_sqlModule->turnGet(turnId));
}

QList<EvalModule::Trace> EvalModule::tracesGet(const QString &conversationId) const {
    const auto messages = m_sqlModule->conversationGet(conversationId).second;
    QList<Trace> traces{};
    QList<SqlModule::Message> turn{};
    for (const auto &message: messages) {
        if (!turn.isEmpty() && turn.constFirst().turnId != message.turnId) {
            const auto trace = traceBuild(turn);
            if (!trace.turnId.isEmpty()) traces.append(trace);
            turn.clear();
        }
        turn.append(message);
    }
    const auto trace = traceBuild(turn);
    if (!trace.turnId.isEmpty()) traces.append(trace);
    return traces;
}

EvalModule::Trace EvalModule::traceBuild(const QList<SqlModule::Message> &messages) {
    if (messages.isEmpty() || messages.constFirst().strategy == AgentModule::AgentStrategy::Team) return {};

    const auto &first = messages.constFirst();
    Trace trace{
        .turnId = first.turnId,
        .conversationId = first.conversationId,
        .provider = first.provider,
        .model = first.model,
        .strategy = first.strategy,
        .status = first.status,
        .error = first.error,
        .timing = first.timing
    };
    for (const auto &message: messages) {
        if (message.role == "assistant") {
            ++trace.modelCalls;
            trace.usage.promptTokens += message.usage.promptTokens;
            trace.usage.completionTokens += message.usage.completionTokens;
            trace.usage.cacheHitTokens += message.usage.cacheHitTokens;
            trace.usage.reasoningTokens += message.usage.reasoningTokens;
            if (trace.timing.firstOutputAt == 0 && message.timing.firstOutputAt > 0) trace.timing.firstOutputAt = message.timing.firstOutputAt;
        } else if (message.role == "tool") {
            ++trace.toolCalls;
            if (message.timing.startedAt > 0 && message.timing.finishedAt > message.timing.startedAt) {
                trace.toolDuration += message.timing.finishedAt - message.timing.startedAt;
            }
        }
        if (message.timing.finishedAt > trace.timing.finishedAt) trace.timing.finishedAt = message.timing.finishedAt;
    }
    return trace;
}
