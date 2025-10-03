#include "../include/diagnostics.h"

// Diagnostics public
Diagnostics::Diagnostics(QWidget *parent)
    : QDockWidget("diagnostics", parent),
      m_diagnosticsTabWidget(new QTabWidget()),
      m_diagnosticsColor{
          {SEVERITY_ERROR, QColor(255, 230, 230)},
          {SEVERITY_WARNING, QColor(255, 245, 230)},
          {SEVERITY_INFO, QColor(230, 240, 250)},
          {SEVERITY_HINT, QColor(245, 245, 245)}
      } {
    setWidget(m_diagnosticsTabWidget);
    m_diagnosticsTabWidget->setMovable(true);
    m_diagnosticsTabWidget->setTabsClosable(true);
    connect(m_diagnosticsTabWidget, &QTabWidget::tabCloseRequested, this, [this](const int index) { m_diagnosticsTabWidget->removeTab(index); });
}

void Diagnostics::diagnosticsReturn(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    if (!m_diagnosticsTableHash.contains(scriptUrl)) {
        // create new table
        QTableWidget *diagnosticsTable;
        diagnosticsTable = new QTableWidget(); // NOLINT
        diagnosticsTable->setColumnCount(5);
        diagnosticsTable->setHorizontalHeaderLabels({"source", "code", "data", "message", "view"});
        diagnosticsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        diagnosticsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        diagnosticsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        diagnosticsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        diagnosticsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        diagnosticsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        diagnosticsTable->verticalHeader()->setVisible(false);
        diagnosticsTable->verticalHeader()->setDefaultSectionSize(24);
        diagnosticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        connect(diagnosticsTable, &QTableWidget::cellClicked, this, [this, diagnosticsTable](const int row, const int column) {
            if (column == 4) {
                const QUrl &url = diagnosticsTable->item(row, column)->data(Qt::UserRole + 1).toUrl();
                const int startLine = diagnosticsTable->item(row, column)->data(Qt::UserRole + 2).toInt();
                const int startCharacter = diagnosticsTable->item(row, column)->data(Qt::UserRole + 3).toInt();
                const int endLine = diagnosticsTable->item(row, column)->data(Qt::UserRole + 4).toInt();
                const int endCharacter = diagnosticsTable->item(row, column)->data(Qt::UserRole + 5).toInt();
                emit highlightScriptAnnotate(url, startLine, startCharacter, endLine, endCharacter);
            }
        });
        m_diagnosticsTableHash.insert(scriptUrl, diagnosticsTable);
    }
    // publish diagnostics
    if (diagnosticsArray.isEmpty()) {
        diagnosticsRemove(scriptUrl);
    } else {
        diagnosticsPublish(scriptUrl, diagnosticsArray);
    }
}

// Diagnostics private
void Diagnostics::diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    QTableWidget *diagnosticsTable = m_diagnosticsTableHash[scriptUrl];
    // check if tab exists
    if (const int index = m_diagnosticsTabWidget->indexOf(diagnosticsTable); index == -1) {
        m_diagnosticsTabWidget->addTab(diagnosticsTable, scriptUrl.fileName());
    }
    m_diagnosticsTabWidget->setCurrentWidget(diagnosticsTable);
    diagnosticsTable->setRowCount(0);
    int row = 0;
    for (const auto &diagnostic: diagnosticsArray) {
        const QJsonObject diagnosticObject = diagnostic.toObject();
        // range
        const QJsonObject diagnosticRange = diagnosticObject["range"].toObject();
        const QJsonObject diagnosticStartPos = diagnosticRange["start"].toObject();
        const QJsonObject diagnosticEndPos = diagnosticRange["end"].toObject();
        const int startLine = diagnosticStartPos["line"].toInt();
        const int startCharacter = diagnosticStartPos["character"].toInt();
        const int endLine = diagnosticEndPos["line"].toInt();
        const int endCharacter = diagnosticEndPos["character"].toInt();
        // information
        const int severity = diagnosticObject["severity"].toInt();
        const QString source = diagnosticObject["source"].toString();
        const QString code = diagnosticObject["code"].toString();
        const QString data = diagnosticObject["data"].toString();
        const QString message = diagnosticObject["message"].toString();
        diagnosticsTable->insertRow(row);
        auto *sourceItem = new QTableWidgetItem(source); // NOLINT
        auto *codeItem = new QTableWidgetItem(code); // NOLINT
        auto *dataItem = new QTableWidgetItem(data); // NOLINT
        auto *messageItem = new QTableWidgetItem(message); // NOLINT
        auto *viewItem = new QTableWidgetItem(); // NOLINT
        viewItem->setIcon(QIcon(":/icon/arrowRight.svg"));
        viewItem->setData(Qt::UserRole + 1, scriptUrl);
        viewItem->setData(Qt::UserRole + 2, startLine);
        viewItem->setData(Qt::UserRole + 3, startCharacter);
        viewItem->setData(Qt::UserRole + 4, endLine);
        viewItem->setData(Qt::UserRole + 5, endCharacter);
        switch (severity) {
            case SEVERITY_ERROR:
                sourceItem->setBackground(m_diagnosticsColor[severity]);
                codeItem->setBackground(m_diagnosticsColor[severity]);
                dataItem->setBackground(m_diagnosticsColor[severity]);
                messageItem->setBackground(m_diagnosticsColor[severity]);
                viewItem->setBackground(m_diagnosticsColor[severity]);
                break;
            case SEVERITY_WARNING:
                sourceItem->setBackground(m_diagnosticsColor[severity]);
                codeItem->setBackground(m_diagnosticsColor[severity]);
                dataItem->setBackground(m_diagnosticsColor[severity]);
                messageItem->setBackground(m_diagnosticsColor[severity]);
                viewItem->setBackground(m_diagnosticsColor[severity]);
                break;
            case SEVERITY_INFO:
                sourceItem->setBackground(m_diagnosticsColor[severity]);
                codeItem->setBackground(m_diagnosticsColor[severity]);
                dataItem->setBackground(m_diagnosticsColor[severity]);
                messageItem->setBackground(m_diagnosticsColor[severity]);
                viewItem->setBackground(m_diagnosticsColor[severity]);
                break;
            case SEVERITY_HINT:
                sourceItem->setBackground(m_diagnosticsColor[severity]);
                codeItem->setBackground(m_diagnosticsColor[severity]);
                dataItem->setBackground(m_diagnosticsColor[severity]);
                messageItem->setBackground(m_diagnosticsColor[severity]);
                viewItem->setBackground(m_diagnosticsColor[severity]);
                break;
            default: break;
        }
        diagnosticsTable->setItem(row, 0, sourceItem);
        diagnosticsTable->setItem(row, 1, codeItem);
        diagnosticsTable->setItem(row, 2, dataItem);
        diagnosticsTable->setItem(row, 3, messageItem);
        diagnosticsTable->setItem(row, 4, viewItem);
        row++;


        // connect(m_scriptDiagnosticsTableWidget, &QTableWidget::cellDoubleClicked, this, [this](const int row, const int col) {
        //     QVariantList pos = m_scriptDiagnosticsTableWidget->item(row, 0)->data(Qt::UserRole + 1).toList();
        //     const int startLine = pos[0].toInt();
        //     const int startCharacter = pos[1].toInt();
        //     const int endLine = pos[2].toInt();
        //     const int endCharacter = pos[3].toInt();
        //     m_currentScriptWidget->m_scriptEditor->setCursorPosition(startLine, startCharacter);
        //     m_currentScriptWidget->m_scriptEditor->fillIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
        //     QTimer::singleShot(1000, [this, startLine, startCharacter, endLine, endCharacter] {
        //         m_currentScriptWidget->m_scriptEditor->clearIndicatorRange(startLine, startCharacter, endLine, endCharacter, INDICATOR_HIGHLIGHT);
        //     });
        // });
        // codeItem->setData(Qt::UserRole + 1, QVariantList({startLine, startCharacter, endLine, endCharacter}));
    }

    // qDebug() << m_diagnosticsTableHash;
}

void Diagnostics::diagnosticsRemove(const QUrl &scriptUrl) {
    // find table
    const auto diagnosticsTable = m_diagnosticsTableHash[scriptUrl];
    // remove hash
    m_diagnosticsTableHash.remove(scriptUrl);
    // remove tab
    const int index = m_diagnosticsTabWidget->indexOf(diagnosticsTable);
    m_diagnosticsTabWidget->removeTab(index);
    // delete table
    diagnosticsTable->deleteLater();

    qDebug() << m_diagnosticsTableHash;
}
