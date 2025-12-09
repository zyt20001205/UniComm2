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
    // load breakpoints
    for (const auto &url: g_breakpoints.keys()) {
        const auto breakpointLineHash = g_breakpoints[url];
        for (auto it = breakpointLineHash.begin(); it != breakpointLineHash.end(); ++it) {
            const int line = it.key();
            const QVariantHash breakpointInfo = it.value();
            breakpointInsert(url, line);
        }
    }
    m_breakpointWidget->rootContext()->setContextProperty("breakpointModule", this);
    m_breakpointWidget->rootContext()->setContextProperty("standardModel", m_breakpointStandardModel);
    m_breakpointWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_breakpointWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/breakpointModule.qml"));
}

void BreakpointModule::breakpointInsert(const QUrl &scriptUrl, const int line) const {
    // update g_breakpoints
    g_breakpoints[scriptUrl][line]["expr"] = "";
    // update model
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
    // update g_breakpoints
    g_breakpoints[scriptUrl].remove(line);
    if (g_breakpoints[scriptUrl].isEmpty()) g_breakpoints.remove(scriptUrl);
    // update model
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

void BreakpointModule::markerRemove(const QUrl &scriptUrl, const int line) {
    emit removeMarker(scriptUrl, MARKER_BREAKPOINT, line - 1);
}
