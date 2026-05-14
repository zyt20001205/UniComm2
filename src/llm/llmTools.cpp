#include "llm/llmTools.h"

#include "globals.h"
#include "document/documentModule.h"

// public
LLMTools::LLMTools(QObject* parent)
    : QObject(parent),
      m_tools{
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
                      {"description", "Get the text content of a specified range in a file. To read the entire document, set all four line and character positional parameters to -1."},
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
                      {"description", "Set the text for a specified range in a document. To overwrite the entire document, set all four line and character positional parameters to -1."},
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
                      {"description", "Start a new thread to execute a script or a specific block of code in a document. To execute the entire document, set all four line and character positional parameters to -1."},
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
          }
      } {
}

QString LLMTools::toolsSet(const QString& name, const QString &arguments) {
    // qDebug() << "name" << name;
    // qDebug() << "arguments" << arguments;
    if (name == "document_list") {
        const auto keys = g_document->documentList();
        QJsonArray array{};
        for (const auto& key : keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    if (name == "document_focused") {
        return g_document->documentFocused();
    }
    if (name == "text_get") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter  = object.value("start_character").toInt(-1);
        const auto endLine  = object.value("end_line").toInt(-1);
        const auto endCharacter  = object.value("end_character").toInt(-1);
        return g_document->textGet(documentUrl, startLine, startCharacter, endLine, endCharacter);
    }
    if (name == "text_set") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto text = object.value("text").toString();
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter  = object.value("start_character").toInt(-1);
        const auto endLine  = object.value("end_line").toInt(-1);
        const auto endCharacter  = object.value("end_character").toInt(-1);
        g_document->textSet(documentUrl, text, startLine, startCharacter, endLine, endCharacter);
        return {"\"Text set finished.\"}"};
    }
    if (name == "thread_start") {
        const auto doc = QJsonDocument::fromJson(arguments.toUtf8());
        const auto object = doc.object();
        const auto documentUrl = QUrl(object.value("document_url").toString());
        const auto mode = object.value("mode").toInt(InterpreterMode::Run);
        const auto startLine = object.value("start_line").toInt(-1);
        const auto startCharacter  = object.value("start_character").toInt(-1);
        const auto endLine  = object.value("end_line").toInt(-1);
        const auto endCharacter  = object.value("end_character").toInt(-1);
        emit g_document->startThread(documentUrl, mode, startLine, startCharacter, endLine, endCharacter);
        return {"\"Thread start finished.\"}"};
    }
    return {};
}
