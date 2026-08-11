#include "agent/role/baseAgent.h"

#include <utility>

#include "agent/module/toolsModule.h"

// public
BaseAgent::BaseAgent(QString id, QObject *parent)
    : QObject(parent),
      m_role(std::move(id)),
      m_systems{
          {
              "supervisor",
              "You are the supervisor and primary IDE code assistant.\n\n"
              "Delegate focused port discovery, configuration, creation, and deletion tasks to the hardware agent with dispatch_agent. Give it a complete, self-contained task and use its final result to continue helping the user.\n\n"
              "For tasks that require multiple implementation or investigation steps, call plan_update before starting substantive work and keep the plan current as work progresses. Do not create a plan for simple tasks.\n\n"
              "If required information is missing or ambiguous and cannot be determined reliably with available tools, call request_user_input instead of guessing. Investigate with tools first and ask one concise question at a time. If the user disables further questions, continue using your best judgment and do not call request_user_input again during that turn.\n\n"
              "Prefer direct tools when available. If no suitable direct tool exists, consult the API annotations and generate a script.\n\n"
              "All generated code must use English for comments, variable names, identifiers, and string literals. In UniComm scripts, use io.log() instead of print()."
          },
          {
              "hardware",
              "You are a hardware engineer responsible for UniComm ports.\n\n"
              "Complete only the delegated port task. Use port_list to inspect existing ports and port_config_get before creating a port. Never invent port names, serial devices, or configuration fields. If required information is still missing, call request_user_input instead of guessing. Return a concise final result to the supervisor."
          }
      },
      m_tools{
          {
              "supervisor",
              {
                  "api_list",
                  "api_get",
                  "demo_get",
                  "database_list",
                  "datatable_list",
                  "dispatch_agent",
                  "plan_update",
                  "request_user_input",
                  "diagnostics_get",
                  "grep_search",
                  "document_list",
                  "document_focused",
                  "line_get",
                  "line_set",
                  "memory_search",
                  "thread_start"
              }
          },
          {
              "hardware",
              {
                  "port_list",
                  "port_config_get",
                  "port_create",
                  "port_delete",
                  "request_user_input"
              }
          }
      } {
}

QString BaseAgent::roleGet() const {
    return m_role;
}

QString BaseAgent::systemGet() const {
    return m_systems.value(m_role);
}

QJsonArray BaseAgent::toolsGet(const ToolsModule &toolsModule) const {
    return toolsModule.toolsGet(m_tools.value(m_role));
}

bool BaseAgent::planRequired(const qsizetype) const {
    return false;
}
