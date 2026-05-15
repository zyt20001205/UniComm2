#include "llm/llmTools.h"

#include <QDir>

#include "globals.h"
#include "data/databaseModule.h"
#include "data/datatableModule.h"
#include "document/documentModule.h"
#include "port/portModule.h"

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
                                              {"description", "The execution mode (e.g., Run, Debug). Pass the corresponding integer enum."}
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
      } {
}

QString LLMTools::toolsSet(const QString &name, const QString &arguments) {
    QString content{};
    QString status{};
    // qDebug() << "name" << name;
    // qDebug() << "arguments" << arguments;
    if (name == "api_list") {
        const auto dir = QDir(":/lib");
        QJsonArray array{};
        const auto entries = dir.entryInfoList();
        for (const auto &entry: entries) {
            array.append(entry.baseName());
        }
        content = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    } else if (name == "api_get") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto packageName = object.value("package_name").toString();
        auto file = QFile(QString(":/lib/%1.d.lua").arg(packageName));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Package '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        content = stream.readAll();
    } else if (name == "demo_get") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto packageName = object.value("package_name").toString();
        auto file = QFile(QString(":/demo/%1.lua").arg(packageName));
        if (!file.open(QIODevice::ReadOnly)) {
            return QString("Demo '%1' not found.").arg(packageName);
        }
        QTextStream stream(&file);
        content = stream.readAll();
    } else if (name == "diagnostics_get") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto diagnostics = g_document->diagnosticsGet(documentUrl);
        content = QString::fromUtf8(QJsonDocument(diagnostics).toJson(QJsonDocument::Compact));
    } else if (name == "document_list") {
        const auto keys = g_document->documentList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        content = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    } else if (name == "document_focused") {
        content = g_document->documentFocused();
    } else if (name == "text_get") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        content = g_document->textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
    } else if (name == "text_set") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto text = object.value("text").toString();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        g_document->textSet(documentUrl, text, startLine, startCharacter, endLine, endCharacter);
        content = "\"Text set finished.\"}";
    } else if (name == "thread_start") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto mode = object.value("mode").toInt(InterpreterMode::Run);
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter = object.value("start_character").toInt(-1);
        const auto endLine = object.value("end_line").toInt(-1);
        const auto endCharacter = object.value("end_character").toInt(-1);
        emit g_document->startThread(documentUrl, mode, startLine, startCharacter, endLine, endCharacter);
        content = "\"Thread start finished.\"}";
    } else if (name == "database_list") {
        const auto keys = g_database->databaseList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        content = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    } else if (name == "datatable_list") {
        const auto keys = g_datatable->datatableList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        content = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    } else if (name == "port_list") {
        const auto keys = g_port->portList();
        QJsonArray array{};
        for (const auto &key: keys) {
            array.append(key);
        }
        content = QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    emit appendChat("tool", name, status);
    return content;
}
