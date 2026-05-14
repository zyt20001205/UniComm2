#include "llm/llmTools.h"

#include "globals.h"
#include "document/documentModule.h"

// public
LLMTools::LLMTools(QObject *parent)
    : QObject(parent),
      m_tools{
          QJsonObject{
              {"type", "function"},
              {
                  "function", QJsonObject{
                      {"name", "documentList"},
                      {"description", "Get the list of files that are currently open in the editor."},
                      {
                          "parameters", QJsonObject{
                              {"type", "object"},
                              {"properties", QJsonObject{}},
                              {"required", QJsonArray{}}
                          }
                      }
                  }
              }
          }
      } {
}

QString LLMTools::toolsSet(const QString& name) {
    if (name == "documentList") {
        const auto keys = g_document->documentList();
        QJsonArray array{};
        for (const auto &key : keys) {
            array.append(key);
        }
        return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
    }
    return {};
}
