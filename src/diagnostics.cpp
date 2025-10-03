#include "../include/diagnostics.h"

Diagnostics::Diagnostics(QWidget *parent)
    : QDockWidget("diagnostics", parent),
      m_diagnosticsTableWidget(new QTableWidget()),
      m_diagnosticsColor{
          {SEVERITY_ERROR, QColor(255, 230, 230)},
          {SEVERITY_WARNING, QColor(255, 245, 230)},
          {SEVERITY_INFO, QColor(230, 240, 250)},
          {SEVERITY_HINT, QColor(245, 245, 245)}
      } {
    setWidget(m_diagnosticsTableWidget);
}

void Diagnostics::diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    m_diagnosticsHash.insert(scriptUrl, diagnosticsArray);
}

void Diagnostics::diagnosticsPublish(const QJsonArray &diagnosticsArray) const {
    m_diagnosticsTableWidget->setRowCount(0);
    int row = 0;
    for (const auto &diagnostic: diagnosticsArray) {
        const QJsonObject diagnosticObject = diagnostic.toObject();
        const int severity = diagnosticObject["severity"].toInt();
        const QJsonObject diagnosticRange = diagnosticObject["range"].toObject();
        const QJsonObject diagnosticStartPos = diagnosticRange["start"].toObject();
        const QJsonObject diagnosticEndPos = diagnosticRange["end"].toObject();
        const int startLine = diagnosticStartPos["line"].toInt();
        const int startCharacter = diagnosticStartPos["character"].toInt();
        const int endLine = diagnosticEndPos["line"].toInt();
        const int endCharacter = diagnosticEndPos["character"].toInt();
        const QString code = diagnosticObject["code"].toString();
        const QString message = diagnosticObject["message"].toString();
        m_diagnosticsTableWidget->insertRow(row);
        auto *codeItem = new QTableWidgetItem(code); // NOLINT
        codeItem->setData(Qt::UserRole + 1, QVariantList({startLine, startCharacter, endLine, endCharacter}));
        auto *messageItem = new QTableWidgetItem(message); // NOLINT
        switch (severity) {
            case SEVERITY_ERROR:
                codeItem->setBackground(m_diagnosticsColor[SEVERITY_ERROR]);
                messageItem->setBackground(m_diagnosticsColor[SEVERITY_ERROR]);
                break;
            case SEVERITY_WARNING:
                codeItem->setBackground(m_diagnosticsColor[SEVERITY_WARNING]);
                messageItem->setBackground(m_diagnosticsColor[SEVERITY_WARNING]);
                break;
            case SEVERITY_INFO:
                codeItem->setBackground(m_diagnosticsColor[SEVERITY_INFO]);
                messageItem->setBackground(m_diagnosticsColor[SEVERITY_INFO]);
                break;
            case SEVERITY_HINT:
                codeItem->setBackground(m_diagnosticsColor[SEVERITY_HINT]);
                messageItem->setBackground(m_diagnosticsColor[SEVERITY_HINT]);
                break;
            default: break;
        }
        m_diagnosticsTableWidget->setItem(row, 0, codeItem);
        m_diagnosticsTableWidget->setItem(row, 1, messageItem);
        row++;
    }
}
