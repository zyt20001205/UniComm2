#include "agent/module/toolsModule.h"

#include <QDir>

#include "globals.h"
#include "agent/agentModule.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"
#include "runtime/threadpoolModule.h"
#include "service/ripgrep.h"

using AgentState = AgentModule::AgentState;

// public
ToolsModule::ToolsModule(QObject *parent)
    : QObject(parent),
      m_writeGroup{"text_set"},
      m_godGroup{"thread_start"},
      m_eventloop(new QEventLoop(this)) {
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
                        "Read text from a document using a range. "
                        "For text files, use start_line/start_character/end_line/end_character = -1/-1/-1/-1 to read the whole document, use x/0/x/-1 to read only line x. "
                        "For PDF files, one call can read only one page, use start_line/start_character/end_line/end_character = x/-1/-1/-1 to read only page x. "
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
                                                "The starting line number (0-based). Pass -1 to start from the beginning of the file. "
                                                "For PDF files, this is the page index (0-based)."
                                            }
                                        }
                                    },
                                    {
                                        "start_character", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The starting character offset within the start line (0-based). Pass -1 to start from the first character. "
                                                "For PDF files, pass -1."
                                            }
                                        }
                                    },
                                    {
                                        "end_line", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The ending line number (0-based, inclusive). Pass -1 to read until the end of the file. "
                                                "For PDF files, pass -1."
                                            }
                                        }
                                    },
                                    {
                                        "end_character", QJsonObject{
                                            {"type", "integer"},
                                            {
                                                "description",
                                                "The ending character offset within the end line. Pass -1 to read until the end of the line. "
                                                "For PDF files, pass -1."
                                            }
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url",
                                    "start_line",
                                    "start_character",
                                    "end_line",
                                    "end_character"
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
                        "Write text to a document using a range. "
                        "Use start_line/start_character/end_line/end_character = -1/-1/-1/-1 to replace the whole document. "
                        "To replace only line x, use x/0/x/-1."
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
                                            {"description", "The new text to insert or replace with."}
                                        }
                                    },
                                    {
                                        "start_line", QJsonObject{
                                            {"type", "integer"},
                                            {"description", "The starting line number (0-based). Pass -1 to start from the beginning of the file."}
                                        }
                                    },
                                    {
                                        "start_character", QJsonObject{
                                            {"type", "integer"},
                                            {"description", "The starting character offset within the start line (0-based). Pass -1 to start from the first character."}
                                        }
                                    },
                                    {
                                        "end_line", QJsonObject{
                                            {"type", "integer"},
                                            {"description", "The ending line number (0-based, inclusive). Pass -1 to replace until the end of the file."}
                                        }
                                    },
                                    {
                                        "end_character", QJsonObject{
                                            {"type", "integer"},
                                            {"description", "The ending character offset within the end line. Pass -1 to replace until the end of the line."}
                                        }
                                    }
                                }
                            },
                            {
                                "required", QJsonArray{
                                    "document_url",
                                    "text",
                                    "start_line",
                                    "start_character",
                                    "end_line",
                                    "end_character"
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

QString ToolsModule::toolsCall(const QString &mode, const QString &name, const QString &arguments) {
    const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
    const auto object = doc.object();
    chatCreate(name, object);
    if (!permissionGet(mode, name, object)) return {"User denied permission to execute this tool."};
    const QDir uniCommDir(QDir(QCoreApplication::applicationDirPath())
                               .filePath("lua-language-server/meta/3rd/UniComm"));
    const QDir apiDir(uniCommDir.filePath("library"));
    const QDir demoDir(uniCommDir.filePath("demo"));
    // UniComm tools
    if (name == "api_list") {
        QJsonArray array{};
        const auto entries = apiDir.entryInfoList({"*.d.lua"}, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const auto &entry: entries) {
            const auto packageName = entry.fileName().chopped(QStringLiteral(".d.lua").size());
            if (!QStringList({"mqtt"}).contains(packageName)) {
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
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        return g_document->textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
    }
    if (name == "text_set") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto text = object.value("text").toString();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        g_document->textSet(documentUrl, text, startLine, startCharacter, endLine, endCharacter);
        return "\"Text set finished.\"}";
    }
    if (name == "thread_start") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        QString threadId{};
        const auto result = g_thread->threadStart(documentUrl, InterpreterMode::Agent, threadId);
        return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
    }
    // MCP tools
    return {"Unknown tool."};
}

void ToolsModule::chatCreate(const QString &name, const QJsonObject &object) {
    QString chatText{};
    if (name == "api_list") {
        chatText = "Get available APIs";
    } else if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        chatText = QString("Read %1 details").arg(packageName);
    } else if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        chatText = QString("Read %1 demo").arg(packageName);
    } else if (name == "grep_search") {
        const auto pattern = object.value("pattern").toString();
        chatText = QString("Grep \"%1\"").arg(pattern);
    } else if (name == "database_list") {
        chatText = "Get available database keys";
    } else if (name == "datatable_list") {
        chatText = "Get available datatable keys";
    } else if (name == "port_list") {
        chatText = "Get available ports";
    } else if (name == "log_get") {
        const auto blockCount = object.value("block_count").toInt(-1);
        chatText = QString("Get last %1 log blocks").arg(QString::number(blockCount));
    } else if (name == "diagnostics_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Check %1 diagnostics").arg(documentName);
    } else if (name == "symbol_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Understand %1 symbol").arg(documentName);
    } else if (name == "document_list") {
        chatText = "Get available documents";
    } else if (name == "document_focused") {
        chatText = "Get current document";
    } else if (name == "text_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        chatText = QString("Read %1 (%2:%3)-(%4:%5)").arg(documentName, QString::number(startLine), QString::number(startCharacter), QString::number(endLine),
                                                          QString::number(endCharacter));
    } else if (name == "text_set") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        chatText = QString("Write %1 (%2:%3)-(%4:%5)").arg(documentName, QString::number(startLine), QString::number(startCharacter), QString::number(endLine),
                                                           QString::number(endCharacter));
    } else if (name == "thread_start") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        chatText = QString("Run %1").arg(documentName);
    }
    emit createChat("tool", chatText);
}

void ToolsModule::permissionSet(const bool status) {
    m_approved = status;
    m_eventloop->quit();
}

bool ToolsModule::permissionGet(const QString &mode, const QString &name, const QJsonObject &object) {
    m_approved = true;
    if (mode == "read") {
        if (m_writeGroup.contains(name) || m_godGroup.contains(name)) m_approved = false;
    } else if (mode == "write") {
        if (m_godGroup.contains(name)) m_approved = false;
    }
    if (m_approved) {
        emit appendChat("", " ✓");
    } else {
        statusSet(name, object);
        m_eventloop->exec();
    }
    emit setState(AgentState::Response, QVariant());
    return m_approved;
}

void ToolsModule::statusSet(const QString &name, const QJsonObject &object) {
    QString statusText{};
    if (name == "api_list") {
        statusText = "I want to get all available APIs.";
    } else if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        statusText = QString("I want to read %1 details.").arg(packageName);
    } else if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        statusText = QString("I want to see %1 demo.").arg(packageName);
    } else if (name == "grep_search") {
        const auto pattern = object.value("pattern").toString();
        statusText = QString("I want to grep \"%1\".").arg(pattern);
    } else if (name == "database_list") {
        statusText = "I want to get all available database keys.";
    } else if (name == "datatable_list") {
        statusText = "I want to get all available datatable keys.";
    } else if (name == "port_list") {
        statusText = "I want to get all available ports.";
    } else if (name == "log_get") {
        const auto blockCount = object.value("block_count").toInt(-1);
        statusText = QString("I want to read latest %1 log blocks.").arg(QString::number(blockCount));
    } else if (name == "diagnostics_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        statusText = QString("I want to check %1 diagnostics.").arg(documentName);
    } else if (name == "symbol_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        statusText = QString("I want to understand %1 symbol.").arg(documentName);
    } else if (name == "document_list") {
        statusText = "I want to get all available documents.";
    } else if (name == "document_focused") {
        statusText = "I want to know current document.";
    } else if (name == "text_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        statusText = QString("I want to read %1 (%2:%3)-(%4:%5).").arg(documentName, QString::number(startLine), QString::number(startCharacter), QString::number(endLine),
                                                                       QString::number(endCharacter));
    } else if (name == "text_set") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        statusText = QString("I want to edit %1 (%2:%3)-(%4:%5).").arg(documentName, QString::number(startLine), QString::number(startCharacter), QString::number(endLine),
                                                                       QString::number(endCharacter));
    } else if (name == "thread_start") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        statusText = QString("I want to run %1.").arg(documentName);
    }
    emit setState(AgentState::Permission, statusText);
}
