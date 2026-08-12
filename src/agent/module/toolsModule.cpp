#include "agent/module/toolsModule.h"

#include <QDir>
#include <QJsonDocument>
#include <QPromise>
#include <QSharedPointer>

#include "globals.h"
#include "agent/agentModule.h"
#include "agent/module/sqlModule.h"
#include "agent/runtime/runtimeModule.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"
#include "runtime/threadpoolModule.h"
#include "service/ripgrep.h"

// public
ToolsModule::ToolsModule(SqlModule *sqlModule, QObject *parent)
    : QObject(parent),
      m_portTypes{
          {"serial_port", PortType::SerialPort},
          {"tcp_client", PortType::TcpClient},
          {"ssl_client", PortType::SslClient}
      },
      m_sqlModule(sqlModule),
      m_writeGroup{"line_set", "port_create"},
      m_fullAccessGroup{"port_delete", "script_exec"} {
}

void ToolsModule::initialize() {
    m_tools = QJsonArray{
        // apiList
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "api_list"},
                    {"description", "Get the list of all available API packages/modules that can be queried for detailed annotations."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // apiGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "api_get"},
                    {"description", "Get the detailed API annotations (function signatures, types, comments) for a specific package."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "package_name", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The name of the package/module to query. Use api_list first to get available names."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{"package_name"}
                            }
                        }
                    }
                }
            }
        },
        // demoGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "demo_get"},
                    {"description", "Get the detailed API demo for a specific package."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "package_name", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The name of the package/module to query. Use api_list first to get available names."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{"package_name"}
                            }
                        }
                    }
                }
            }
        },
        // databaseList
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "database_list"},
                    {"description", "Get the list of all available database keys that can be queried for detailed annotations."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // datatableList
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "datatable_list"},
                    {"description", "Get the list of all available data table keys that can be queried for detailed annotations."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // subagentDispatch
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "subagent_dispatch"},
                    {"description", "Delegate one or more independent tasks to specialized agents, run them concurrently, and wait for all final results."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "tasks", QJsonObject{
                                            {"type", "array"},
                                            {"description", "Independent tasks that can run concurrently."},
                                            {
                                                "items", QJsonObject{
                                                    {"type", "object"},
                                                    {
                                                        "properties", QJsonObject{
                                                            {
                                                                "role", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"enum", QJsonArray{"hardware", "software"}},
                                                                    {"description", "The specialized agent role."}
                                                                }
                                                            },
                                                            {
                                                                "task", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"description", "A complete, self-contained task for the specialized agent."}
                                                                }
                                                            }
                                                        }
                                                    },
                                                    {"required", QJsonArray{"role", "task"}}
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"tasks"}}
                        }
                    }
                }
            }
        },
        // planUpdate
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "plan_update"},
                    {
                        "description",
                        "Create or update the execution plan for a complex task. Submit the complete plan on every update, keep at most one step in progress, and mark steps completed only after the work is finished. Do not use this tool for simple one-step tasks."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "explanation", QJsonObject{
                                            {"type", "string"},
                                            {"description", "A concise user-facing summary of the current progress or why the plan changed."}
                                        }
                                    },
                                    {
                                        "plan", QJsonObject{
                                            {"type", "array"},
                                            {
                                                "items", QJsonObject{
                                                    {"type", "object"},
                                                    {
                                                        "properties", QJsonObject{
                                                            {
                                                                "step", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"description", "A concise actionable step."}
                                                                }
                                                            },
                                                            {
                                                                "status", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"enum", QJsonArray{"pending", "in_progress", "completed"}}
                                                                }
                                                            }
                                                        }
                                                    },
                                                    {"required", QJsonArray{"step", "status"}}
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"plan"}}
                        }
                    }
                }
            }
        },
        // userInputRequest
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "user_input_request"},
                    {
                        "description",
                        "Ask the user one concise question only when the missing information cannot be discovered with tools and choosing incorrectly would materially affect the result. Investigate first and do not ask for information that tools can provide. Offer up to three mutually exclusive suggested answers when useful."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "question", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The concise question to show the user."}
                                        }
                                    },
                                    {
                                        "options", QJsonObject{
                                            {"type", "array"},
                                            {"description", "Zero to three mutually exclusive suggested answers, with the recommended option first. Use an empty array when the user should provide free-form input."},
                                            {
                                                "items", QJsonObject{
                                                    {"type", "object"},
                                                    {
                                                        "properties", QJsonObject{
                                                            {
                                                                "label", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"description", "The concise answer returned when this option is selected."}
                                                                }
                                                            },
                                                            {
                                                                "description", QJsonObject{
                                                                    {"type", "string"},
                                                                    {"description", "A short explanation of the option's impact or meaning."}
                                                                }
                                                            }
                                                        }
                                                    },
                                                    {"required", QJsonArray{"label", "description"}}
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"question", "options"}}
                        }
                    }
                }
            }
        },
        // portList
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "port_list"},
                    {"description", "Get the list of all available ports that can be queried for detailed annotations."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // portConfigGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "port_config_get"},
                    {"description", "Get the required fields, defaults, constraints, and available options for creating a port of the specified type."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "port_type", QJsonObject{
                                            {"type", "string"},
                                            {"enum", QJsonArray{"serial_port", "tcp_client", "ssl_client"}},
                                            {"description", "The type of port to configure."}
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"port_type"}}
                        }
                    }
                }
            }
        },
        // portInsert
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "port_create"},
                    {"description", "Create a configured port. Call port_config_get first and pass a configuration matching the returned definition."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "port_type", QJsonObject{
                                            {"type", "string"},
                                            {"enum", QJsonArray{"serial_port", "tcp_client", "ssl_client"}},
                                            {"description", "The type of port to create."}
                                        }
                                    },
                                    {
                                        "config", QJsonObject{
                                            {"type", "object"},
                                            {"description", "The type-specific configuration returned by port_config_get."}
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"port_type", "config"}}
                        }
                    }
                }
            }
        },
        // portRemove
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "port_delete"},
                    {"description", "Delete an existing configured port by name. Call port_list first to get the available port names."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "port_name", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The name of the port to delete."}
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"port_name"}}
                        }
                    }
                }
            }
        },
        // diagnosticsGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "diagnostics_get"},
                    {
                        "description",
                        "Get the diagnostics for a specified document. Returns an array of diagnostic items. If the array is empty, there are no diagnostics (i.e., no errors or warnings). Errors related to 'PLACEHOLDER' should be ignored as they are expected placeholders."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "document_url", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The URL / file path of the document."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url"
                                }
                            }
                        }
                    }
                }
            }
        },
        // grepSearch
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "grep_search"},
                    {"description", "Search text across files in the current workspace."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "pattern", QJsonObject{
                                            {"type", "string"},
                                            {"description", "Text or regex pattern to search for."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "pattern"
                                }
                            }
                        }
                    }
                }
            }
        },
        // documentList
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "document_list"},
                    {"description", "Get the list of documents that are currently open in the editor."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // documentFocused
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "document_focused"},
                    {"description", "Get the currently focused document in the editor."},
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{}},
                            {"required", QJsonArray{}}
                        }
                    }
                }
            }
        },
        // linesGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "line_get"},
                    {
                        "description",
                        "Read lines from a text document. Each returned line is prefixed with its 0-based line number in the form line|content. "
                        "Use start_line = 0 and line_count = -1 to read the whole document. A line_count of -1 reads from start_line to the end of the document. "
                        "If the requested range extends past the end of the document, all remaining lines are returned."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "document_url", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The URL / file path of the document."}
                                        }
                                    },
                                    {
                                        "start_line", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The starting line number (0-based). "
                                                "For PDF files, this is the page index (0-based)."
                                            }
                                        }
                                    },
                                    {
                                        "line_count", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The number of lines to read. Pass -1 to read to the end of a text document. "
                                                "For PDF files, this value is ignored."
                                            }
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url",
                                    "start_line",
                                    "line_count"
                                }
                            }
                        }
                    }
                }
            }
        },
        // linesSet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "line_set"},
                    {
                        "description",
                        "Replace the content of whole lines in a text document. "
                        "A line_count of -1 replaces from start_line to the end of the document."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "document_url", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The URL / file path of the document."}
                                        }
                                    },
                                    {
                                        "start_line", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The starting line number (0-based)."
                                            }
                                        }
                                    },
                                    {
                                        "line_count", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The number of existing lines to replace. Pass -1 to replace from start_line to the end of the document."
                                            }
                                        }
                                    },
                                    {
                                        "expected", QJsonObject{
                                            {"type", "string"},
                                            {
                                                "description",
                                                "The current content of start_line without the line number and | prefix."
                                            }
                                        }
                                    },
                                    {
                                        "text", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The replacement text without line-number prefixes. Pass an empty string to clear the target lines."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url",
                                    "start_line",
                                    "line_count",
                                    "expected",
                                    "text"
                                }
                            }
                        }
                    }
                }
            }
        },
        // memorySearch
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "memory_search"},
                    {
                        "description",
                        "Search prior conversations in the current workspace when historical decisions, requirements, errors, or implementation details may help with the current task. Returns complete conversation turns around the best keyword matches."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "query", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The FTS5 keyword query used to search conversation memory."}
                                        }
                                    },
                                    {
                                        "limit", QJsonObject{
                                            {"type", "integer"},
                                            {"description", "The maximum number of matching turns to return."},
                                            {"minimum", 1},
                                            {"maximum", 5}
                                        }
                                    }
                                }
                            },
                            {"required", QJsonArray{"query"}}
                        }
                    }
                }
            }
        },
        // scriptExec
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "script_exec"},
                    {
                        "description",
                        "Execute the specified script. Before execution, you must first call diagnostics_get to verify that there are no syntax errors or warnings."
                    },
                    {
                        "parameters", QJsonObject{
                            {"type", "object"},
                            {
                                "properties", QJsonObject{
                                    {
                                        "document_url", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The URL / file path of the document to execute."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url"
                                }
                            }
                        }
                    }
                }
            }
        },
    };
}

QJsonArray ToolsModule::toolsGet(const QSet<QString> &names) const {
    QJsonArray tools{};
    for (const auto &value: m_tools) {
        const auto name = value.toObject().value("function").toObject().value("name").toString();
        if (names.contains(name)) tools.append(value);
    }
    return tools;
}

QPair<bool, QString> ToolsModule::toolCall(const int mode, const QString &name, const QString &arguments) const {
    return {permissionGet(mode, name), toolTextGet(name, arguments)};
}

QFuture<QString> ToolsModule::toolExecute(const QString &runtimeId, const QString &name, const QString &arguments) {
    const auto object = QJsonDocument::fromJson(arguments.toUtf8()).object();
    if (name == "subagent_dispatch") {
        const auto tasks = object.value("tasks").toArray();
        if (tasks.isEmpty()) return QtFuture::makeReadyValueFuture(QString("No subagent tasks were provided."));
        auto promise = QSharedPointer<QPromise<QString>>::create();
        auto results = QSharedPointer<QJsonArray>::create();
        auto remaining = QSharedPointer<qsizetype>::create(tasks.size());
        for (qsizetype index = 0; index < tasks.size(); ++index) results->append(QJsonObject{});
        promise->start();
        const auto future = promise->future();
        const auto finish = [promise, results, remaining](const qsizetype index, const QString &role, const QString &result) {
            (*results)[index] = QJsonObject{{"role", role}, {"result", result}};
            if (--*remaining > 0) return;
            promise->addResult(QString::fromUtf8(QJsonDocument(*results).toJson(QJsonDocument::Compact)));
            promise->finish();
        };
        for (qsizetype index = 0; index < tasks.size(); ++index) {
            const auto task = tasks.at(index).toObject();
            const auto role = task.value("role").toString();
            auto *worker = g_agent->subagentDispatch(role, task.value("task").toString());
            if (worker == nullptr) {
                finish(index, role, QString("Unknown agent role: %1").arg(role));
                continue;
            }
            connect(worker, &RuntimeModule::finishRun, this, [finish, index, role](const QString &result) {
                finish(index, role, result);
            });
        }
        return future;
    }
    if (name == "script_exec") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        auto threadId = QSharedPointer<QString>::create();
        auto promise = QSharedPointer<QPromise<QString>>::create();
        auto connection = QSharedPointer<QMetaObject::Connection>::create();
        promise->start();
        const auto future = promise->future();
        *connection = connect(g_threadpool, &ThreadpoolModule::finishThread, this, [threadId, promise, connection](const QString &id, const QJsonArray &result) {
            if (id != *threadId) return;
            disconnect(*connection);
            promise->addResult(QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact)));
            promise->finish();
        });
        g_threadpool->threadStart(documentUrl, InterpreterMode::Agent, *threadId);
        return future;
    }
    return QtFuture::makeReadyValueFuture(toolExecuteSync(runtimeId, name, object));
}

QString ToolsModule::toolExecuteSync(const QString &runtimeId, const QString &name, const QJsonObject &object) {
    const QDir uniCommDir(QDir(QCoreApplication::applicationDirPath()).filePath("lua-language-server/meta/3rd/UniComm"));
    const QDir apiDir(uniCommDir.filePath("library"));
    const QDir demoDir(uniCommDir.filePath("demo"));
    // UniComm tools
    if (name == "api_list") {
        QJsonArray array{};
        const auto entries = apiDir.entryInfoList({"*.d.lua"}, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const auto &entry: entries) {
            const auto packageName = entry.fileName().chopped(QStringLiteral(".d.lua").size());
            if (!QStringList({"mqtt", "types"}).contains(packageName)) {
                array.append(packageName);
            }
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        if (packageName.isEmpty() || packageName.contains('/') || packageName.contains('\\') || packageName.contains("..")) {
            return "Invalid package name.";
        }
        auto file = QFile(apiDir.filePath(packageName + ".d.lua"));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Package '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        return stream.readAll();
    }
    if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        if (packageName.isEmpty() || packageName.contains('/') || packageName.contains('\\') || packageName.contains("..")) {
            return "Invalid package name.";
        }
        auto file = QFile(demoDir.filePath(packageName + ".lua"));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Demo '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        return stream.readAll();
    }
    if (name == "database_list") {
        const auto keys = g_database->databaseList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "datatable_list") {
        const auto keys = g_datatable->datatableList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "plan_update") {
        if (object.contains("explanation") && !object.value("explanation").isString()) return "Plan update failed: explanation must be a string.";
        if (!object.value("plan").isArray()) return "Plan update failed: plan must be an array.";

        QJsonArray normalizedSteps{};
        auto inProgressCount = 0;
        auto completedCount = 0;
        for (const auto &value: object.value("plan").toArray()) {
            if (!value.isObject()) return "Plan update failed: every plan item must be an object.";

            const auto stepObject = value.toObject();
            const auto description = stepObject.value("step").toString().trimmed();
            const auto status = stepObject.value("status").toString();
            if (description.isEmpty()) return "Plan update failed: step must be a non-empty string.";
            if (status == "in_progress") ++inProgressCount;
            else if (status == "completed") ++completedCount;
            else if (status != "pending") return "Plan update failed: status must be pending, in_progress, or completed.";

            normalizedSteps.append(QJsonObject{
                {"step", description},
                {"status", status}
            });
        }
        if (normalizedSteps.isEmpty()) return "Plan update failed: plan must contain at least one step.";
        if (inProgressCount > 1) return "Plan update failed: at most one step can be in progress.";

        const QJsonObject plan{
            {"explanation", object.value("explanation").toString()},
            {"plan", normalizedSteps}
        };
        g_agent->planUpdate(runtimeId, plan);
        return QString("Plan updated: %1/%2 steps completed.").arg(completedCount).arg(normalizedSteps.size());
    }
    if (name == "port_list") {
        const auto keys = g_port->portList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "port_config_get") {
        const auto portType = m_portTypes.value(object.value("port_type").toString(), -1);
        const auto config = PortModule::portConfigGet(portType);
        if (config.isEmpty()) return "Unsupported port type.";
        return QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact));
    }
    if (name == "port_create") {
        const auto portType = m_portTypes.value(object.value("port_type").toString(), -1);
        auto config = object.value("config").toObject();
        config["portType"] = portType;
        return g_port->portInsert(-1, config);
    }
    if (name == "port_delete") {
        return g_port->portRemove(object.value("port_name").toString());
    }
    if (name == "diagnostics_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto diagnostics = g_document->diagnosticsGet(documentUrl);
        return QString::fromUtf8(QJsonDocument(diagnostics).toJson(QJsonDocument::Compact));
    }
    if (name == "grep_search") {
        const auto pattern = object.value("pattern").toString();
        const auto result = g_ripgrep->grep(pattern);
        return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
    }
    if (name == "document_list") {
        const auto keys = g_document->documentList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "document_focused") {
        return g_document->documentFocused();
    }
    if (name == "line_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto documentInfo = QFileInfo(documentUrl.toLocalFile());
        if (!documentInfo.isFile()) return "Line get failed: document does not exist.";
        const auto startLine = object.value("start_line").toInt();
        const auto lineCount = object.value("line_count").toInt();
        if (startLine < 0) return {"Line get failed: start_line is out of range."};
        if (lineCount == 0 || lineCount < -1) return {"Line get failed: line_count is out of range."};

        const auto text = g_document->linesGet(documentUrl, startLine, lineCount);
        if (text.isNull()) return {"Line get failed: start_line is out of range."};
        auto lines = text.split('\n', Qt::KeepEmptyParts);
        for (qsizetype i = 0; i < lines.size(); ++i) lines[i].prepend(QString::number(startLine + i) + "|");
        return lines.join('\n');
    }
    if (name == "line_set") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto documentInfo = QFileInfo(documentUrl.toLocalFile());
        if (!documentInfo.isFile()) return "Line set failed: document does not exist.";
        const auto startLine = object.value("start_line").toInt();
        const auto lineCount = object.value("line_count").toInt();
        const auto expected = object.value("expected").toString().section('\n', 0, 0);
        const auto text = object.value("text").toString();
        if (startLine < 0) return {"Line set failed: start_line is out of range."};
        if (lineCount == 0 || lineCount < -1) return {"Line set failed: line_count is out of range."};
        if (g_document->linesGet(documentUrl, startLine, 1).trimmed() != expected.trimmed()) return {"Line set failed: document changed. Call line_get and retry."};

        g_document->linesSet(documentUrl, text, startLine, lineCount);
        return {"Line set finished."};
    }
    if (name == "memory_search") {
        const auto limit = qBound(1, object.value("limit").toInt(3), 5);
        const auto results = m_sqlModule->conversationsSearch(object.value("query").toString(), limit);
        QJsonArray array{};
        for (const auto &result: results) {
            QJsonArray messages{};
            for (const auto &message: result.messages) {
                if (message.content.isEmpty()) continue;
                messages.append(QJsonObject{
                    {"role", message.role},
                    {"content", message.content}
                });
            }
            array.append(QJsonObject{
                {"conversation_id", result.conversationId},
                {"conversation_title", result.conversationTitle},
                {"turn_id", result.turnId},
                {"created_at", result.createdAt},
                {"rank", result.rank},
                {"messages", messages}
            });
        }
        const auto json = QJsonDocument(array);
        return QString::fromUtf8(json.toJson(QJsonDocument::Compact));
    }
    return {"Unknown tool."};
}

QString ToolsModule::toolTextGet(const QString &name, const QString &arguments) const {
    const auto object = QJsonDocument::fromJson(arguments.toUtf8()).object();
    QString chatText{};
    if (name == "api_list") {
        chatText = "List available APIs";
    } else if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        chatText = QString("Read %1 API details").arg(packageName);
    } else if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        chatText = QString("Read %1 demo").arg(packageName);
    } else if (name == "grep_search") {
        const auto pattern = object.value("pattern").toString();
        chatText = QString("Grep \"%1\"").arg(pattern);
    } else if (name == "database_list") {
        chatText = "List available databases";
    } else if (name == "datatable_list") {
        chatText = "List available datatables";
    } else if (name == "subagent_dispatch") {
        const auto tasks = object.value("tasks").toArray();
        chatText = tasks.size() == 1
                       ? QString("Delegate task to %1 agent").arg(tasks.first().toObject().value("role").toString())
                       : QString("Delegate %1 tasks to subagents").arg(tasks.size());
    } else if (name == "plan_update") {
        chatText = "Update plan";
    } else if (name == "user_input_request") {
        chatText = object.value("question").toString();
    } else if (name == "port_list") {
        chatText = "List available ports";
    } else if (name == "port_config_get") {
        const auto portType = object.value("port_type").toString();
        chatText = QString("Get %1 port configuration").arg(portType);
    } else if (name == "port_create") {
        const auto portName = object.value("config").toObject().value("portName").toString();
        chatText = QString("Create port %1").arg(portName);
    } else if (name == "port_delete") {
        const auto portName = object.value("port_name").toString();
        chatText = QString("Delete port %1").arg(portName);
    } else if (name == "log_get") {
        const auto blockCount = object.value("block_count").toInt(-1);
        chatText = QString("Get last %1 log blocks").arg(QString::number(blockCount));
    } else if (name == "diagnostics_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Check diagnostics for %1").arg(documentName);
    } else if (name == "document_list") {
        chatText = "List open documents";
    } else if (name == "document_focused") {
        chatText = "Get focused document";
    } else if (name == "line_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto lineCount = object.value("line_count").toInt(-1);
        chatText = lineCount == -1
                       ? QString("Read %1 from line %2 to the end").arg(documentName, QString::number(startLine))
                       : QString("Read %1 from line %2 (%3 lines)").arg(documentName, QString::number(startLine), QString::number(lineCount));
    } else if (name == "line_set") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto lineCount = object.value("line_count").toInt(-1);
        chatText = lineCount == -1
                       ? QString("Write %1 from line %2 to the end").arg(documentName, QString::number(startLine))
                       : QString("Write %1 from line %2 (%3 lines)").arg(documentName, QString::number(startLine), QString::number(lineCount));
    } else if (name == "memory_search") {
        chatText = QString("Search memory for \"%1\"").arg(object.value("query").toString());
    } else if (name == "script_exec") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Run %1").arg(documentName);
    }
    return chatText.isEmpty() ? name : chatText;
}

bool ToolsModule::permissionGet(const int mode, const QString &name) const {
    switch (mode) {
        case RuntimeModule::AgentMode::Chat: return false;
        case RuntimeModule::AgentMode::Read: return !m_writeGroup.contains(name) && !m_fullAccessGroup.contains(name);
        case RuntimeModule::AgentMode::Write: return !m_fullAccessGroup.contains(name);
        case RuntimeModule::AgentMode::FullAccess: return true;
        default: return false;
    }
}
