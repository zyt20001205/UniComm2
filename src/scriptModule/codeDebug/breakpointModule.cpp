#include "scriptModule/codeDebug/breakpointModule.h"

#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"

// BreakpointModule public
BreakpointModule::BreakpointModule()
    : DockWidget("breakpoint"),
      m_breakpointConfig(g_workspaceConfig["breakpointConfig"].toObject()),
      m_breakpointWidget(new QQuickWidget()),
      m_breakpointStandardModel(new QStandardItemModel()) {
    setWidget(m_breakpointWidget);
    for (const auto &key: m_breakpointConfig.keys()) {
        const QUrl url(key);
        const auto breakpointLineHash = m_breakpointConfig[key].toObject();
        for (auto it = breakpointLineHash.begin(); it != breakpointLineHash.end(); ++it) {
            const int line = it.key().toInt();
            const QVariantHash breakpointInfo = it.value().toObject().toVariantHash();
            g_breakpoints[url].insert(line, breakpointInfo);
            breakpointInsert(url, line);
        }
    }
}

void BreakpointModule::breakpointConfigSave() {
    auto breakpointHash = QJsonObject();
    for (const auto &url: g_breakpoints.keys()) {
        auto breakpointLineHash = QJsonObject();
        for (auto it = g_breakpoints[url].begin(); it != g_breakpoints[url].end(); ++it) {
            const int line = it.key();
            const QVariantHash &info = it.value();
            breakpointLineHash.insert(QString::number(line), QJsonObject::fromVariantHash(info));
        }
        breakpointHash.insert(url.toString(), breakpointLineHash);
    }
    g_workspaceConfig["breakpointConfig"] = breakpointHash;
}

void BreakpointModule::propertySet(const QVariantMap &objects) {
    m_conditionDialog = qvariant_cast<QObject *>(objects["breakpointModuleConditionDialog"]);
    m_lineMenu = qvariant_cast<QObject *>(objects["breakpointModuleLineMenu"]);
    m_fileMenu = qvariant_cast<QObject *>(objects["breakpointModuleFileMenu"]);
    m_rootMenu = qvariant_cast<QObject *>(objects["breakpointModuleRootMenu"]);
    m_breakpointWidget->rootContext()->setContextProperty("lineMenu", m_lineMenu);
    m_breakpointWidget->rootContext()->setContextProperty("fileMenu", m_fileMenu);
    m_breakpointWidget->rootContext()->setContextProperty("rootMenu", m_rootMenu);
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

void BreakpointModule::breakpointDelete(const QUrl &scriptUrl, const int line) {
    breakpointRemove(scriptUrl, line);
    emit removeMarker(scriptUrl, MARKER_BREAKPOINT, line - 1);
}

void BreakpointModule::breakpointsDelete(const QUrl &scriptUrl) {
    const auto *indent0 = m_breakpointStandardModel->invisibleRootItem();
    for (int i = 0; i < indent0->rowCount(); ++i) {
        auto *indent1 = indent0->child(i);
        if (indent1->data(Qt::WhatsThisRole).toUrl() == scriptUrl) {
            for (int j = indent1->rowCount() - 1; j >= 0; --j) {
                const auto *indent2 = indent1->child(j);
                const auto line = indent2->text().toInt();
                breakpointDelete(scriptUrl, line);
            }
        }
    }
}

QString BreakpointModule::conditionGet(const QUrl &scriptUrl, int line) {
}

void BreakpointModule::conditionSet(const QUrl &scriptUrl, int line, const QString &condition) {
}

void BreakpointModule::allDelete() {
    const auto *indent0 = m_breakpointStandardModel->invisibleRootItem();
    for (int i = indent0->rowCount() - 1; i >= 0; --i) {
        auto *indent1 = indent0->child(i);
        const auto scriptUrl = indent1->data(Qt::WhatsThisRole).toUrl();
        breakpointsDelete(scriptUrl);
    }
}
