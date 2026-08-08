#include "agent/module/contextModule.h"

#include <QJsonObject>

#include "agent/agentModule.h"

ContextModule::ContextModule(QObject *parent)
    : QObject(parent),
      m_system("You are an IDE code assistant.\n\n"
          "%1\n\n"
          "For tasks that require multiple implementation or investigation steps, call plan_update before starting substantive work and keep the plan current as work progresses. Do not create a plan for simple tasks.\n\n"
          "If required information is missing or ambiguous and cannot be determined reliably with available tools, call request_user_input instead of guessing. Investigate with tools first and ask one concise question at a time. If the user disables further questions, continue using your best judgment and do not call request_user_input again during that turn.\n\n"
          "Prefer direct tools when available. If no suitable direct tool exists, consult the API annotations and generate a script.\n\n"
          "All generated code must use English for comments, variable names, identifiers, and string literals. In UniComm scripts, use io.log() instead of print().") {
}

QJsonArray ContextModule::contextBuild(const int mode, const QList<SqlModule::Message> &history, const QList<SqlModule::Message> &turn) const {
    auto messages = history;
    messages.append(turn);

    QString modePrompt{};
    switch (mode) {
        case AgentModule::AgentMode::Chat:
            modePrompt = "Current operating mode: chat.\n\n"
                    "No tools are available in this mode. Answer using only the conversation and information provided by the user. If the task requires inspecting, modifying, or executing anything in the workspace, ask the user to switch to an appropriate agent mode.";
            break;
        case AgentModule::AgentMode::Read:
            modePrompt = "Current operating mode: read.\n\n"
                    "You may inspect the workspace and use read-only tools. Do not modify workspace data, execute programs, or perform operations that change external state.";
            break;
        case AgentModule::AgentMode::Write:
            modePrompt = "Current operating mode: write.\n\n"
                    "You may inspect and modify workspace data using read and write tools. Do not execute programs or use operations reserved for full-access mode.";
            break;
        case AgentModule::AgentMode::FullAccess:
            modePrompt = "Current operating mode: full-access.\n\n"
                    "You may use all available tools, including program execution and operations that modify or delete data. Use these capabilities only when necessary for the user's request, and keep all actions narrowly scoped.";
            break;
        default: break;
    }

    QJsonArray context{
        QJsonObject{
            {"role", "system"},
            {"content", m_system.arg(modePrompt)}
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
