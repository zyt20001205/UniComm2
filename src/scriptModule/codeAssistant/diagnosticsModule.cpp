#include "scriptModule/codeAssistant/diagnosticsModule.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>

#include "globals.h"

// DiagnosticsModule public
DiagnosticsModule::DiagnosticsModule()
    : DockWidget("diagnostics"),
      m_diagnosticsWidget(new QQuickWidget()),
      m_diagnosticsTabWidget(new QTabWidget()),
      m_diagnosticsColor{
          {LEVEL_ERROR, QColor(255, 230, 230)},
          {LEVEL_WARNING, QColor(255, 245, 230)},
          {LEVEL_INFO, QColor(230, 240, 250)},
          {LEVEL_HINT, QColor(245, 245, 245)}
      } {
    setWidget(m_diagnosticsWidget);
    // m_diagnosticsWidget->rootContext()->setContextProperty("structureModule", this);
    // m_diagnosticsWidget->rootContext()->setContextProperty("standardModel", m_structureStandardModel);
    m_diagnosticsWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_diagnosticsWidget->setSource(QUrl("qrc:/qml/scriptModule/codeAssistant/diagnosticsModule.qml"));
    m_diagnosticsRoot = m_diagnosticsWidget->rootObject();

    // setWidget(m_diagnosticsTabWidget);
    // m_diagnosticsTabWidget->setMovable(true);
    // m_diagnosticsTabWidget->setTabsClosable(true);
    // connect(m_diagnosticsTabWidget, &QTabWidget::tabCloseRequested, this, [this](const int index) { diagnosticsClose(index); });
}

void DiagnosticsModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    if (diagnostics.isEmpty()) {
        diagnosticsRemove(scriptUrl);
    } else {
        diagnosticsPublish(scriptUrl, diagnostics);
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

void DiagnosticsModule::diagnosticsPublish(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    QVariantList diagnosticsParsed{};
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        // range
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject startPos = range["start"].toObject();
        const QJsonObject endPos = range["end"].toObject();
        const int startLine = startPos["line"].toInt();
        const int startCharacter = startPos["character"].toInt();
        const int endLine = endPos["line"].toInt();
        const int endCharacter = endPos["character"].toInt();
        // information
        const QString severity = QString::number(diagnostic["severity"].toInt());
        const QString source = diagnostic["source"].toString();
        const QString code = diagnostic["code"].toString();
        const QString data = diagnostic["data"].toString();
        const QString message = diagnostic["message"].toString();
        diagnosticsParsed.append(QVariantMap{
            {"severity", severity},
            {"source", source},
            {"code", code},
            {"data", data},
            {"message", message},
            // {"startLine", startLine},
            // {"startCharacter", startCharacter},
            // {"endLine", endLine},
            // {"endCharacter", endCharacter}
        });
    }
    QMetaObject::invokeMethod(m_diagnosticsRoot, "append", Q_ARG(QVariant, scriptUrl.fileName()), Q_ARG(QVariant, diagnosticsParsed));


    // // qDebug() << diagnostics;
    // QTableWidget *diagnosticsTable = m_diagnosticsTableHash[scriptUrl];
    // // check if tab exists
    // if (diagnosticsTable == nullptr) {
    //     // create diagnostics table
    //     diagnosticsTable = new QTableWidget();
    //     m_diagnosticsTableHash[scriptUrl] = diagnosticsTable;
    //     diagnosticsTable->setColumnCount(5);
    //     diagnosticsTable->setHorizontalHeaderLabels({"Source", "Code", "Data", "Message", "View"});
    //     diagnosticsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    //     diagnosticsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    //     diagnosticsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    //     diagnosticsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    //     diagnosticsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    //     diagnosticsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    //     diagnosticsTable->verticalHeader()->setVisible(false);
    //     diagnosticsTable->verticalHeader()->setDefaultSectionSize(24);
    //     diagnosticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //     connect(diagnosticsTable, &QTableWidget::cellClicked, this, [this, diagnosticsTable](const int row, const int column) {
    //         if (column == 4) {
    //             const QUrl &url = diagnosticsTable->item(row, column)->data(Qt::UserRole + 1).toUrl();
    //             const int startLine = diagnosticsTable->item(row, column)->data(Qt::UserRole + 2).toInt();
    //             const int startCharacter = diagnosticsTable->item(row, column)->data(Qt::UserRole + 3).toInt();
    //             const int endLine = diagnosticsTable->item(row, column)->data(Qt::UserRole + 4).toInt();
    //             const int endCharacter = diagnosticsTable->item(row, column)->data(Qt::UserRole + 5).toInt();
    //             emit openScript(url);
    //             emit setCursorPosition(url, startLine, startCharacter);
    //             emit insertIndicator(url, INDICATOR_SELECTION, startLine, startCharacter, endLine, endCharacter, 1000);
    //         }
    //     });
    //     m_diagnosticsTabWidget->addTab(diagnosticsTable, scriptUrl.fileName());
    // }
    // m_diagnosticsTabWidget->setCurrentWidget(diagnosticsTable);
    // diagnosticsTable->setRowCount(0);
    // int row = 0;
    // for (const auto &value: diagnostics) {
    //     const QJsonObject diagnostic = value.toObject();
    //     // range
    //     const QJsonObject range = diagnostic["range"].toObject();
    //     const QJsonObject startPos = range["start"].toObject();
    //     const QJsonObject endPos = range["end"].toObject();
    //     const int startLine = startPos["line"].toInt();
    //     const int startCharacter = startPos["character"].toInt();
    //     const int endLine = endPos["line"].toInt();
    //     const int endCharacter = endPos["character"].toInt();
    //     // information
    //     const int severity = diagnostic["severity"].toInt();
    //     const QString source = diagnostic["source"].toString();
    //     const QString code = diagnostic["code"].toString();
    //     const QString data = diagnostic["data"].toString();
    //     const QString message = diagnostic["message"].toString();
    //     diagnosticsTable->insertRow(row);
    //     auto *sourceItem = new QTableWidgetItem(source); // NOLINT
    //     auto *codeItem = new QTableWidgetItem(code); // NOLINT
    //     auto *dataItem = new QTableWidgetItem(data); // NOLINT
    //     auto *messageItem = new QTableWidgetItem(message); // NOLINT
    //     auto *viewItem = new QTableWidgetItem(QIcon(":/icon/arrowRight.svg"), ""); // NOLINT
    //     viewItem->setData(Qt::UserRole + 1, scriptUrl);
    //     viewItem->setData(Qt::UserRole + 2, startLine);
    //     viewItem->setData(Qt::UserRole + 3, startCharacter);
    //     viewItem->setData(Qt::UserRole + 4, endLine);
    //     viewItem->setData(Qt::UserRole + 5, endCharacter);
    //
    //     sourceItem->setBackground(m_diagnosticsColor[severity]);
    //     codeItem->setBackground(m_diagnosticsColor[severity]);
    //     dataItem->setBackground(m_diagnosticsColor[severity]);
    //     messageItem->setBackground(m_diagnosticsColor[severity]);
    //     viewItem->setBackground(m_diagnosticsColor[severity]);
    //
    //     diagnosticsTable->setItem(row, 0, sourceItem);
    //     diagnosticsTable->setItem(row, 1, codeItem);
    //     diagnosticsTable->setItem(row, 2, dataItem);
    //     diagnosticsTable->setItem(row, 3, messageItem);
    //     diagnosticsTable->setItem(row, 4, viewItem);
    //     row++;
    // }
    // // qDebug() << m_diagnosticsTableHash;
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
