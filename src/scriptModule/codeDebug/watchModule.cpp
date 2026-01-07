#include "scriptModule/codeDebug/watchModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>

#include "globals.h"

// WatchModule public
WatchModule::WatchModule()
    : DockWidget("Watch"),
      m_watchWidget(new QQuickWidget()) {
    setWidget(m_watchWidget);
    g_watchStandardItemModel = new QStandardItemModel(this);
}

WatchModule::~WatchModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] watch module destructed").arg(timestamp);
}

void WatchModule::propertySet(const QVariantMap &objects) {
    m_watchWidget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["watchModuleTableMenu"]));
    m_watchWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["watchModuleRootMenu"]));

    m_watchWidget->rootContext()->setContextProperty("watchModule", this);
    m_watchWidget->rootContext()->setContextProperty("standardItemModel", g_watchStandardItemModel);
    m_watchWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_watchWidget->setSource(QUrl("qrc:/qml/scriptModule/codeDebug/watchModule.qml"));
    m_rootItem = m_watchWidget->rootObject();
}

void WatchModule::watchConfigSave() const {

}

void WatchModule::watchInsert(int index, const QUrl &scriptUrl, const QString &key) {
    index = g_watchStandardItemModel->rowCount();
    auto *keyItem = new QStandardItem(key); // NOLINT
    keyItem->setData(scriptUrl, Qt::WhatsThisRole);
    auto *valueItem = new QStandardItem(); // NOLINT
    g_watchStandardItemModel->insertRow(index, {keyItem, valueItem});
}

void WatchModule::watchRemove(const int index) {
    g_watchStandardItemModel->removeRow(index);
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

