#include "agent/module/contextModule.h"

#include <QJsonObject>

ContextModule::ContextModule(QObject *parent)
    : QObject(parent),
      m_system("You are an IDE code assistant. "
          "When in chat mode (no tools provided), you can only answer questions. If the request cannot be handled, ask user to switch to agent mode. "
          "When in agent mode (read/write/full-access), you have access to file system, terminal, and advanced tools. "
          "For tasks that require multiple implementation or investigation steps, use plan_update before starting substantive work and keep the plan current as work progresses. Do not create a plan for simple tasks. "
          "Use tools first when possible. If not, consult API annotations and generate a script. "
          "All code must be written in English (including comments, variable names, identifiers, and strings). "
          "Use io.log() instead of print() for assistant.") {
}

QJsonArray ContextModule::contextBuild(const QList<SqlModule::Message> &history, const QList<SqlModule::Message> &turn) const {
    auto messages = history;
    messages.append(turn);

    QJsonArray context{
        QJsonObject{
            {"role", "system"},
            {"content", m_system}
        }
    };
    for (const auto &message: messages) {
        QJsonObject object{
            {"role", message.role},
            {"content", message.content}
        };
        if (!message.reasoningContent.isEmpty()) object["reasoning_content"] = message.reasoningContent;
        if (!message.toolCallId.isEmpty()) object["tool_call_id"] = message.toolCallId;
        if (!message.toolCalls.isEmpty()) object["tool_calls"] = message.toolCalls;
        context.append(object);
    }
    return context;
}
