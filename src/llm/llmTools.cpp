#include "llm/llmTools.h"

#include <QDir>

#include "globals.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"
#include "runtime/threadpoolModule.h"

// public
LLMTools::LLMTools(QObject *parent)
    : QObject(parent),
      m_tools{
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
                          "Get the text content of a specified range in a file. To read the entire document, set all four line and character positional parameters to -1."
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
                                                  "The starting line number (0-based). Pass -1 to start from the beginning of the file."
                                              }
                                          }
                                      },
                                      {
                                          "start_character", QJsonObject{
                                              {"type", "integer"},
                                              {
                                                  "description",
                                                  "The starting character offset within the start line (0-based). Pass -1 to start from the first character."
                                              }
                                          }
                                      },
                                      {
                                          "end_line", QJsonObject{
                                              {"type", "integer"},
                                              {
                                                  "description",
                                                  "The ending line number (0-based, inclusive). Pass -1 to read until the end of the file."
                                              }
                                          }
                                      },
                                      {
                                          "end_character", QJsonObject{
                                              {"type", "integer"},
                                              {
                                                  "description",
                                                  "The ending character offset within the end line. Pass -1 to read until the end of the line."
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
                          "Set the text for a specified range in a document. To overwrite the entire document, set all four line and character positional parameters to -1."
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
                          "Start a new thread to execute a script or a specific block of code in a document. Before execution, you must first call diagnostics_get to verify that there are no syntax errors or warnings. To execute the entire document, set all four line and character positional parameters to -1."
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
                                      },
                                      {
                                          "mode", QJsonObject{
                                              {"type", "integer"},
                                              {"description", "Execution mode for the session. Always use Run (0) unless the user specifically asks for Debug (1)."}
                                          }
                                      },
                                      {
                                          "start_line", QJsonObject{
                                              {"type", "integer"},
                                              {"description", "The starting line number (0-based) of the code block to execute. Pass -1 for the whole file."}
                                          }
                                      },
                                      {
                                          "start_character", QJsonObject{
                                              {"type", "integer"},
                                              {"description", "The starting character offset (0-based). Pass -1 for the whole file."}
                                          }
                                      },
                                      {
                                          "end_line", QJsonObject{
                                              {"type", "integer"},
                                              {"description", "The ending line number (0-based). Pass -1 for the whole file."}
                                          }
                                      },
                                      {
                                          "end_character", QJsonObject{
                                              {"type", "integer"},
                                              {"description", "The ending character offset. Pass -1 for the whole file."}
                                          }
                                      }
                                  }
                              },
                              {
                                  "required", QJsonArray{
                                      "document_url",
                                      "mode",
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
      },
      m_writeGroup{"text_set"},
      m_godGroup{"thread_start"},
      m_eventloop(new QEventLoop(this)) {
}

QString LLMTools::toolsSet(const QString &mode, const QString &name, const QString &arguments) {
    const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
    const auto object = doc.object();
    if (!permissionGet(mode, name, object)) return {"User denied permission to execute this tool."};
    if (name == "api_list") {
        const auto dir = QDir(":/lib");
        QJsonArray array{};
        const auto entries = dir.entryInfoList();
        for (const auto &entry: entries) {
            array.append(entry.baseName());
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        auto file = QFile(QString(":/lib/%1.d.lua").arg(packageName));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Package '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        return stream.readAll();
    }
    if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        auto file = QFile(QString(":/demo/%1.lua").arg(packageName));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Demo '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        return stream.readAll();
    }
    if (name == "diagnostics_get") {
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto diagnostics = g_document->diagnosticsGet(documentUrl);
        return QString::fromUtf8(QJsonDocument(diagnostics).toJson(QJsonDocument::Compact));
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
        const auto mode = object.value("mode").toInt(InterpreterMode::Run);
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        g_thread->threadStart(documentUrl, mode, startLine, startCharacter, endLine, endCharacter);
        return "\"Thread started.\"}";
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
    return {"Unknown tool."};
}

void LLMTools::permissionSet(const bool status) {
    m_approved = status;
    m_eventloop->quit();
}

bool LLMTools::permissionGet(const QString &mode, const QString &name, const QJsonObject &object) {
    m_approved = true;
    if (mode == "read") {
        if (m_writeGroup.contains(name) || m_godGroup.contains(name)) m_approved = false;
    } else if (mode == "write") {
        if (m_godGroup.contains(name)) m_approved = false;
    }
    QString text{};
    QString status{};
    if (name == "api_list") {
        text = "Get available APIs";
        status = "I want to get all available APIs.";
    } else if (name == "api_get") {
        const auto packageName = object.value("package_name").toString();
        text = QString("Read %1 details").arg(packageName);
        status = QString("I want to read %1 details.").arg(packageName);
    } else if (name == "demo_get") {
        const auto packageName = object.value("package_name").toString();
        text = QString("Read %1 demo").arg(packageName);
        status = QString("I want to see %1 demo.").arg(packageName);
    } else if (name == "diagnostics_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        text = QString("Check %1 diagnostics").arg(documentName);
        status = QString("I want to check %1 diagnostics.").arg(documentName);
    } else if (name == "document_list") {
        text = "Get available documents";
        status = "I want to get all available documents.";
    } else if (name == "document_focused") {
        text = "Get current document";
        status = "I want to know current document.";
    } else if (name == "text_get") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        text = QString("Read %1").arg(documentName);
        status = QString("I want to read %1.").arg(documentName);
    } else if (name == "text_set") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        text = QString("Write %1").arg(documentName);
        status = QString("I want to edit %1.").arg(documentName);
    } else if (name == "thread_start") {
        const auto documentName = QUrl(object.value("document_url").toString()).fileName();
        text = QString("Run %1").arg(documentName);
        status = QString("I want to run %1.").arg(documentName);
    } else if (name == "database_list") {
        text = "Get available database keys";
        status = "I want to get all available database keys.";
    } else if (name == "datatable_list") {
        text = "Get available datatable keys";
        status = "I want to get all available datatable keys.";
    } else if (name == "port_list") {
        text = "Get available ports";
        status = "I want to get all available ports.";
    }
    if (m_approved) text += " ✓";
    emit appendChat("tool", text, status);
    if (!m_approved) m_eventloop->exec();
    emit appendChat("user", "", "Responding...");
    return m_approved;
}
