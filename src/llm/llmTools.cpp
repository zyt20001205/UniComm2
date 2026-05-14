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
                      {"description", "Get the text content of a specified range in a file."},
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
    return {};
}
