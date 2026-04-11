#include "debug/watchModule.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"

// public
WatchModule::WatchModule()
    : DockWidget("Watch"),
      m_watchWidget(new QQuickWidget()) {
    setWidget(m_watchWidget);
    g_watchStandardItemModel = new QStandardItemModel(this);
    auto watchConfig = g_workspaceConfig["watchConfig"].toArray();
    for (const auto &value : watchConfig) {
        const auto pair = value.toArray();
        const auto documentUrl = QUrl(pair[0].toString());
        const QString expression = pair[1].toString();
        watchInsert(-1, documentUrl, expression);
    }
}

WatchModule::~WatchModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] watch module destructed").arg(timestamp);
}

void WatchModule::propertySet(const QVariantMap &objects) {
    m_watchWidget->rootContext()->setContextProperty("expressionMenu", qvariant_cast<QObject *>(objects["watchModuleExpressionMenu"]));
    m_watchWidget->rootContext()->setContextProperty("valueMenu", qvariant_cast<QObject *>(objects["watchModuleValueMenu"]));
    m_watchWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["watchModuleRootMenu"]));

    m_watchWidget->rootContext()->setContextProperty("watchModule", this);
    m_watchWidget->rootContext()->setContextProperty("standardItemModel", g_watchStandardItemModel);
    m_watchWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_watchWidget->setSource(QUrl("qrc:/qml/debug/watchModule.qml"));
    m_rootItem = m_watchWidget->rootObject();
}

void WatchModule::watchConfigSave() const {
    auto watchArray = QJsonArray();
    for (int i = 0; i < g_watchStandardItemModel->rowCount(); ++i) {
        const QString url = g_watchStandardItemModel->item(i, 0)->data(Qt::WhatsThisRole).toString();
        const QString expression = g_watchStandardItemModel->item(i, 0)->text();
        watchArray.append(QJsonArray({url, expression}));
    }
    g_workspaceConfig["watchConfig"] = watchArray;
}

void WatchModule::watchInsert(int index, const QUrl &documentUrl, const QString &expression) {
    index = g_watchStandardItemModel->rowCount();
    auto *expressionItem = new QStandardItem(expression); // NOLINT
    expressionItem->setData(documentUrl, Qt::WhatsThisRole);
    auto *valueItem = new QStandardItem("nil"); // NOLINT
    valueItem->setData("nil", Qt::WhatsThisRole);
    g_watchStandardItemModel->insertRow(index, {expressionItem, valueItem});
}

void WatchModule::watchRemove(const int index) {
    g_watchStandardItemModel->removeRow(index);
}

void WatchModule::watchRename(const int index, const QUrl &documentUrl, const QString &expression) {
    g_watchStandardItemModel->item(index, 0)->setData(documentUrl, Qt::WhatsThisRole);
    g_watchStandardItemModel->item(index, 0)->setText(expression);
}

void WatchModule::watchSwap(const int src, const int dst) {
    const auto tmp = g_watchStandardItemModel->takeRow(src);
    g_watchStandardItemModel->insertRow(dst, tmp);
    QMetaObject::invokeMethod(m_rootItem, "reload");
}

void WatchModule::watchClear(const int index) {
    if (index == -1) {
        for (int i = 0; i < g_watchStandardItemModel->rowCount(); ++i) {
            g_watchStandardItemModel->item(i, 1)->setText("");
        }
    } else {
        g_watchStandardItemModel->item(index, 1)->setText("");
    }
}

