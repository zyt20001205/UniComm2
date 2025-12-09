#include "scriptModule/codeDebug/breakpointModule.h"

#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"

// BreakpointModule public
BreakpointModule::BreakpointModule()
    : DockWidget("breakpoint"),
      m_breakpointWidget(new QQuickWidget()),
      m_breakpointStandardModel(new QStandardItemModel()) {
    setWidget(m_breakpointWidget);
    m_breakpointWidget->rootContext()->setContextProperty("breakpointModule", this);
    m_breakpointWidget->rootContext()->setContextProperty("standardModel", m_breakpointStandardModel);
    m_breakpointWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_breakpointWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/breakpointModule.qml"));
}

void BreakpointModule::breakpointInsert(const QUrl &scriptUrl, const int line) const {
    auto *lineItem = new QStandardItem(QString::number(line)); // NOLINT
    lineItem->setData(scriptUrl, Qt::WhatsThisRole);
    const auto *indent0 = m_breakpointStandardModel->invisibleRootItem();
    for (int i = 0; i < indent0->rowCount(); ++i) {
        auto *indent1 = indent0->child(i);
        if (indent1->data(Qt::WhatsThisRole).toUrl() == scriptUrl) {
            for (int j = 0; j < indent1->rowCount(); ++j) {
                const auto *indent2 = indent1->child(j);
                if (line < indent2->text().toInt()) {
                    indent1->insertRow(j, lineItem);
                    return;
                }
            }
            indent1->appendRow(lineItem);
            return;
        }
    }
    auto *urlItem = new QStandardItem(scriptUrl.fileName()); // NOLINT
    urlItem->setData(scriptUrl, Qt::WhatsThisRole);
    urlItem->appendRow(lineItem);
    m_breakpointStandardModel->appendRow(urlItem);
}

void BreakpointModule::breakpointRemove(const QUrl &scriptUrl, const int line) const {
    const auto *indent0 = m_breakpointStandardModel->invisibleRootItem();
    for (int i = 0; i < indent0->rowCount(); ++i) {
        auto *indent1 = indent0->child(i);
        if (indent1->data(Qt::WhatsThisRole).toUrl() == scriptUrl) {
            for (int j = 0; j < indent1->rowCount(); ++j) {
                const auto *indent2 = indent1->child(j);
                if (line == indent2->text().toInt()) {
                    indent1->removeRow(j);
                    if (indent1->rowCount() == 0) {
                        m_breakpointStandardModel->removeRow(i);
                    }
                    return;
                }
            }
        }
    }
}

void BreakpointModule::scriptOpen(const QUrl &scriptUrl) {
    emit openScript(scriptUrl);
}

void BreakpointModule::markerInsert(const QUrl &scriptUrl, const int line) {
    emit openScript(scriptUrl);
    emit insertMarker(scriptUrl, MARKER_HINT, line - 1, 1000);
}
