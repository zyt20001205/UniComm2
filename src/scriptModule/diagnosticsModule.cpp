#include "scriptModule/diagnosticsModule.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QTableWidget>
#include <QTabWidget>

#include "globals.h"

// DiagnosticsModule public
DiagnosticsModule::DiagnosticsModule()
    : DockWidget("diagnostics"),
      m_diagnosticsTabWidget(new QTabWidget()),
      m_diagnosticsColor{
          {LEVEL_ERROR, QColor(255, 230, 230)},
          {LEVEL_WARNING, QColor(255, 245, 230)},
          {LEVEL_INFO, QColor(230, 240, 250)},
          {LEVEL_HINT, QColor(245, 245, 245)}
      } {
    setWidget(m_diagnosticsTabWidget);
    m_diagnosticsTabWidget->setMovable(true);
    m_diagnosticsTabWidget->setTabsClosable(true);
    connect(m_diagnosticsTabWidget, &QTabWidget::tabCloseRequested, this, [this](const int index) { diagnosticsClose(index); });
}

void DiagnosticsModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    if (diagnosticsArray.isEmpty()) {
        diagnosticsRemove(scriptUrl);
    } else {
        diagnosticsPublish(scriptUrl, diagnosticsArray);
    }
}

// DiagnosticsModule private
void DiagnosticsModule::diagnosticsClose(const int index) {
    // find diagnostics table
    const auto *diagnosticsTable = static_cast<QTableWidget *>(m_diagnosticsTabWidget->widget(index));
    // find script url
    QUrl scriptUrl;
    foreach(const QUrl &url, m_diagnosticsTableHash.keys()) {
        if (m_diagnosticsTableHash.value(url) == diagnosticsTable) {
            scriptUrl = url;
            break;
        }
    }
    diagnosticsRemove(scriptUrl);
}

void DiagnosticsModule::diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray) {
    // qDebug() << diagnosticsArray;
    QTableWidget *diagnosticsTable = m_diagnosticsTableHash[scriptUrl];
    // check if tab exists
    if (diagnosticsTable == nullptr) {
        // create diagnostics table
        diagnosticsTable = new QTableWidget();
        m_diagnosticsTableHash[scriptUrl] = diagnosticsTable;
        diagnosticsTable->setColumnCount(5);
        diagnosticsTable->setHorizontalHeaderLabels({"Source", "Code", "Data", "Message", "View"});
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
                emit openScript(url);
                emit setCursorPosition(url, startLine, startCharacter);
                emit insertIndicator(url, INDICATOR_HIGHLIGHT, startLine, startCharacter, endLine, endCharacter, 1000);
            }
        });
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
        auto *viewItem = new QTableWidgetItem(QIcon(":/icon/arrowRight.svg"), ""); // NOLINT
        viewItem->setData(Qt::UserRole + 1, scriptUrl);
        viewItem->setData(Qt::UserRole + 2, startLine);
        viewItem->setData(Qt::UserRole + 3, startCharacter);
        viewItem->setData(Qt::UserRole + 4, endLine);
        viewItem->setData(Qt::UserRole + 5, endCharacter);

        sourceItem->setBackground(m_diagnosticsColor[severity]);
        codeItem->setBackground(m_diagnosticsColor[severity]);
        dataItem->setBackground(m_diagnosticsColor[severity]);
        messageItem->setBackground(m_diagnosticsColor[severity]);
        viewItem->setBackground(m_diagnosticsColor[severity]);

        diagnosticsTable->setItem(row, 0, sourceItem);
        diagnosticsTable->setItem(row, 1, codeItem);
        diagnosticsTable->setItem(row, 2, dataItem);
        diagnosticsTable->setItem(row, 3, messageItem);
        diagnosticsTable->setItem(row, 4, viewItem);
        row++;
    }
    // qDebug() << m_diagnosticsTableHash;
}

void DiagnosticsModule::diagnosticsRemove(const QUrl &scriptUrl) {
    // find table
    const auto diagnosticsTable = m_diagnosticsTableHash[scriptUrl];
    // remove hash
    m_diagnosticsTableHash.remove(scriptUrl);
    // remove tab
    const int index = m_diagnosticsTabWidget->indexOf(diagnosticsTable);
    m_diagnosticsTabWidget->removeTab(index);
    // delete table
    diagnosticsTable->deleteLater();
    // qDebug() << m_diagnosticsTableHash;
}