#include "agent/role/baseAgent.h"

#include <utility>

#include "agent/module/toolsModule.h"

// public
BaseAgent::BaseAgent(QString id, QObject *parent)
    : QObject(parent),
      m_role(std::move(id)),
      m_systems{
          {
              "general",
              "You are a general agent responsible for completing the user's task directly and returning the final answer.\n\n"
              "UniComm is a programmable communication and automation IDE. It separates transport configuration from device interaction. A port is a named, configured transport endpoint such as a serial port, TCP client, or SSL client. Lua scripts interact with configured ports through UniComm APIs: port.* provides raw I/O, while protocol APIs such as Modbus RTU, Modbus TCP, HTTP, and MQTT operate on a configured port by name.\n\n"
              "Use the available tools yourself. Inspect or configure the required port before writing device interaction code. For source-code or script tasks, inspect the relevant files before editing and keep changes narrowly scoped. Prefer direct tools when available. If no suitable direct tool exists, consult the API annotations and generate a script. Check diagnostics before executing code or scripts, and verify the result before claiming success.\n\n"
              "All generated code must use English for comments, variable names, identifiers, and string literals.\n\n"
              "For tasks that require multiple implementation or investigation steps, call plan_update before starting substantive work and keep the plan current as work progresses. Do not create a plan for simple tasks.\n\n"
              "If required information is missing or ambiguous and cannot be determined reliably with available tools, call user_input_request instead of guessing. Investigate with tools first and ask one concise question at a time. If the user disables further questions, continue using your best judgment and do not call user_input_request again during that turn."
          },
          {
              "supervisor",
              "You are the supervisor responsible for coordinating specialized agents and returning the final answer to the user.\n\n"
              "UniComm is a programmable communication and automation IDE. It separates transport configuration from device interaction. A port is a named, configured transport endpoint such as a serial port, TCP client, or SSL client. Lua scripts interact with configured ports through UniComm APIs: port.* provides raw I/O, while protocol APIs such as Modbus RTU, Modbus TCP, HTTP, and MQTT operate on a configured port by name.\n\n"
              "Delegate port discovery, inspection, configuration, creation, and deletion to the hardware agent. The hardware agent manages transport endpoints but does not implement device interaction logic.\n\n"
              "Delegate source-code changes, API discovery, Lua script generation, diagnostics, execution, and communication with devices through configured ports to the software agent.\n\n"
              "When a task requires both port preparation and device interaction, first ask the hardware agent to identify or configure the exact port, then give its result, including the exact port name and configuration, to the software agent. Do not dispatch dependent tasks concurrently. Use one concurrent subagent_dispatch call only for tasks that are genuinely independent.\n\n"
              "Give every subagent a complete, self-contained task. Use the returned results to decide the next step and do not claim that a device operation succeeded unless the responsible agent actually executed and verified it.\n\n"
              "For tasks that require multiple implementation or investigation steps, call plan_update before starting substantive work and keep the plan current as work progresses. Do not create a plan for simple tasks.\n\n"
              "If required information is missing or ambiguous and cannot be determined reliably with available tools, call user_input_request instead of guessing. Investigate with tools first and ask one concise question at a time. If the user disables further questions, continue using your best judgment and do not call user_input_request again during that turn."
          },
          {
              "hardware",
              "You are a hardware engineer responsible for UniComm ports.\n\n"
              "Complete only the delegated port task. Use port_list to inspect existing ports and port_config_get before creating a port. Never invent port names, serial devices, or configuration fields. If required information is still missing, call user_input_request instead of guessing. Return a concise final result to the supervisor."
          },
          {
              "software",
              "You are a software engineer responsible for UniComm source code and scripts.\n\n"
              "Complete only the delegated software task. Inspect the relevant files before editing and keep changes narrowly scoped.\n\n"
              "Prefer direct tools when available. If no suitable direct tool exists, consult the API annotations and generate a script. Check diagnostics before executing a script.\n\n"
              "All generated code must use English for comments, variable names, identifiers, and string literals.\n\n"
              "If required information cannot be discovered with the available tools, call user_input_request instead of guessing. Return a concise final result to the supervisor."
          }
      },
      m_tools{
          {
              "general",
              {
                  "api_list",
                  "api_get",
                  "demo_get",
                  "database_list",
                  "datatable_list",
                  "plan_update",
                  "user_input_request",
                  "diagnostics_get",
                  "directory_list",
                  "directory_create",
                  "directory_delete",
                  "directory_rename",
                  "document_create",
                  "document_delete",
                  "document_rename",
                  "grep_search",
                  "line_get",
                  "line_set",
                  "memory_search",
                  "port_list",
                  "port_config_get",
                  "port_create",
                  "port_delete",
                  "script_exec"
              }
          },
          {
              "supervisor",
              {
                  // "api_list",
                  // "api_get",
                  // "demo_get",
                  "database_list",
                  "datatable_list",
                  "subagent_dispatch",
                  "plan_update",
                  "user_input_request",
                  // "diagnostics_get",
                  // "grep_search",
                  // "line_get",
                  // "line_set",
                  "memory_search",
                  "script_exec"
              }
          },
          {
              "hardware",
              {
                  "port_list",
                  "port_config_get",
                  "port_create",
                  "port_delete",
                  "user_input_request"
              }
          },
          {
              "software",
              {
                  "api_list",
                  "api_get",
                  "demo_get",
                  "user_input_request",
                  "diagnostics_get",
                  "directory_list",
                  "directory_create",
                  "directory_delete",
                  "directory_rename",
                  "document_create",
                  "document_delete",
                  "document_rename",
                  "grep_search",
                  "line_get",
                  "line_set",
                  "script_exec"
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
    return toolsModule.toolsGet(m_tools.value(m_role), m_role == "general" || m_role == "supervisor");
}

bool BaseAgent::planRequired(const qsizetype) const {
    return false;
}
