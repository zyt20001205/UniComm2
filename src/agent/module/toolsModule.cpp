#include "agent/module/toolsModule.h"

#include <QDir>
#include <limits>

#include "globals.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"
#include "runtime/threadpoolModule.h"
#include "service/ripgrep.h"

// public
ToolsModule::ToolsModule(QObject *parent)
    : QObject(parent),
      m_writeGroup{"text_set"},
      m_fullAccessGroup{"thread_start"} {
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
        // symbolGet
        QJsonObject{
            {"type", "function"},
            {
                "function", QJsonObject{
                    {"name", "symbol_get"},
                    {
                        "description",
                        "Get the structural symbols (e.g., classes, functions, variables) of a specified document along with their line numbers."
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

QPair<bool, QString> ToolsModule::toolCall(const QString &mode, const QString &name, const QString &arguments) const {
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
    if (name == "port_list") {
        const auto keys = g_port->portList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "diagnostics_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto diagnostics = g_document->diagnosticsGet(documentUrl);
        return QString::fromUtf8(QJsonDocument(diagnostics).toJson(QJsonDocument::Compact));
    }
    if (name == "symbol_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto symbol = g_document->symbolGet(documentUrl);
        return QString::fromUtf8(QJsonDocument(symbol).toJson(QJsonDocument::Compact));
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
    } else if (name == "port_list") {
        chatText = "List available ports";
    } else if (name == "log_get") {
        const auto blockCount = object.value("block_count").toInt(-1);
        chatText = QString("Get last %1 log blocks").arg(QString::number(blockCount));
    } else if (name == "diagnostics_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Check diagnostics for %1").arg(documentName);
    } else if (name == "symbol_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Inspect symbols in %1").arg(documentName);
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

bool ToolsModule::permissionGet(const QString &mode, const QString &name) const {
    if (mode == "read") return !m_writeGroup.contains(name) && !m_fullAccessGroup.contains(name);
    if (mode == "write") return !m_fullAccessGroup.contains(name);
    return true;
}
