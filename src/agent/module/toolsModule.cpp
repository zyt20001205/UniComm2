#include "agent/module/toolsModule.h"

#include <QDir>
#include <QJsonDocument>
#include <limits>

#include "globals.h"
#include "agent/agentModule.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"
#include "runtime/threadpoolModule.h"
#include "service/ripgrep.h"

// public
ToolsModule::ToolsModule(QObject *parent)
    : QObject(parent),
      m_portTypes{
          {"serial_port", PortType::SerialPort},
          {"tcp_client", PortType::TcpClient},
          {"ssl_client", PortType::SslClient}
      },
      m_writeGroup{"text_set", "port_create"},
      m_fullAccessGroup{"port_delete", "thread_start"} {
}

void ToolsModule::initialize() {
    const auto tools = QJsonArray{
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
        // textGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "text_get"},
                    {
                        "description",
                        "Read lines from a document. For text files, each returned line is prefixed with its 0-based line number in the form line|content. "
                        "Use start_line = 0 and line_count = -1 to read the whole document. A line_count of -1 reads from start_line to the end of the document. "
                        "If the requested range extends past the end of the document, all remaining lines are returned. "
                        "For PDF files, start_line is the page index (0-based) and line_count is ignored."
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
        // textSet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "text_set"},
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
                                        "text", QJsonObject{
                                            {"type", "string"},
                                            {"description", "The replacement text without line-number prefixes. Pass an empty string to clear the target lines."}
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
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url",
                                    "text",
                                    "start_line",
                                    "line_count"
                                }
                            }
                        }
                    }
                }
            }
        },
        // threadStart
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "thread_start"},
                    {
                        "description",
                        "Start a new thread to execute a script. Before execution, you must first call diagnostics_get to verify that there are no syntax errors or warnings."
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
    emit registerTools("UniComm", tools);
}

QPair<bool, QString> ToolsModule::toolCall(const int mode, const QString &name, const QString &arguments) const {
    return {permissionGet(mode, name), toolTextGet(name, arguments)};
}

QString ToolsModule::toolExecute(const QString &name, const QString &arguments) {
    const auto object = QJsonDocument::fromJson(arguments.toUtf8()).object();
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
        emit updatePlan(plan);
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
    if (name == "text_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto documentInfo = QFileInfo(documentUrl.toLocalFile());
        if (!documentInfo.isFile()) return "Text get failed: document does not exist.";
        const auto startLine = object.value("start_line").toInt();
        const auto lineCount = object.value("line_count").toInt();
        if (startLine < 0) return {"Text get failed: start_line is out of range."};
        if (lineCount == 0 || lineCount < -1) return {"Text get failed: line_count is out of range."};

        const auto endLine = lineCount == -1 ? std::numeric_limits<int>::max() : startLine + lineCount - 1;
        auto text = g_document->textGet(documentUrl, startLine, 0, endLine, -1);
        if (text.isEmpty()) return {};
        auto lines = text.split('\n');
        for (qsizetype i = 0; i < lines.size(); ++i) {
            if (lines[i].endsWith('\r')) lines[i].chop(1);
            lines[i].prepend(QString::number(startLine + i) + "|");
        }
        return lines.join('\n');
    }
    if (name == "text_set") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto documentInfo = QFileInfo(documentUrl.toLocalFile());
        if (!documentInfo.isFile()) return "Text set failed: document does not exist.";
        const auto text = object.value("text").toString();
        const auto startLine = object.value("start_line").toInt();
        const auto lineCount = object.value("line_count").toInt();
        if (startLine < 0) return {"Text set failed: start_line is out of range."};
        if (lineCount == 0 || lineCount < -1) return {"Text set failed: line_count is out of range."};

        const auto endLine = lineCount == -1 ? std::numeric_limits<int>::max() : startLine + lineCount - 1;
        g_document->textSet(documentUrl, text, startLine, 0, endLine, -1);
        return {"Text set finished."};
    }
    if (name == "thread_start") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        QString threadId{};
        const auto result = g_thread->threadStart(documentUrl, InterpreterMode::Agent, threadId);
        return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
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
    } else if (name == "plan_update") {
        chatText = "Update plan";
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
    } else if (name == "text_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto lineCount = object.value("line_count").toInt(-1);
        chatText = lineCount == -1
                       ? QString("Read %1 from line %2 to the end").arg(documentName, QString::number(startLine))
                       : QString("Read %1 from line %2 (%3 lines)").arg(documentName, QString::number(startLine), QString::number(lineCount));
    } else if (name == "text_set") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto lineCount = object.value("line_count").toInt(-1);
        chatText = lineCount == -1
                       ? QString("Write %1 from line %2 to the end").arg(documentName, QString::number(startLine))
                       : QString("Write %1 from line %2 (%3 lines)").arg(documentName, QString::number(startLine), QString::number(lineCount));
    } else if (name == "thread_start") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Run %1").arg(documentName);
    }
    return chatText.isEmpty() ? name : chatText;
}

bool ToolsModule::permissionGet(const int mode, const QString &name) const {
    switch (mode) {
        case AgentModule::AgentMode::Chat: return false;
        case AgentModule::AgentMode::Read: return !m_writeGroup.contains(name) && !m_fullAccessGroup.contains(name);
        case AgentModule::AgentMode::Write: return !m_fullAccessGroup.contains(name);
        case AgentModule::AgentMode::FullAccess: return true;
        default: return false;
    }
}
