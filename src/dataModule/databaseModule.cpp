#include "dataModule/databaseModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>

#include "globals.h"

// DatabaseModule public
DatabaseModule::DatabaseModule()
    : DockWidget("Database"),
      m_databaseWidget(new QQuickWidget()) {
    setWidget(m_databaseWidget);
    g_databaseStandardItemModel = new QStandardItemModel(this);
    for (const auto &value: g_workspaceConfig["databaseConfig"].toArray()) {
        const QString key = value.toString();
        databaseInsert(-1, key);
    }
}

DatabaseModule::~DatabaseModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] database module destructed").arg(timestamp);
}

void DatabaseModule::propertySet(const QVariantMap &objects) {
    m_databaseWidget->rootContext()->setContextProperty("editDialog", qvariant_cast<QObject *>(objects["databaseModuleEditDialog"]));
    m_databaseWidget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["databaseModuleTableMenu"]));
    m_databaseWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["databaseModuleRootMenu"]));

    m_databaseWidget->rootContext()->setContextProperty("databaseModule", this);
    m_databaseWidget->rootContext()->setContextProperty("standardItemModel", g_databaseStandardItemModel);
    m_databaseWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_databaseWidget->setSource(QUrl("qrc:/qml/dataModule/databaseModule.qml"));
    m_rootItem = m_databaseWidget->rootObject();
}

void DatabaseModule::databaseConfigSave() const {
    QJsonArray keyArray{};
    for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
        const QString key = g_databaseStandardItemModel->item(i, 0)->text();
        keyArray.append(key);
    }
    g_workspaceConfig["databaseConfig"] = keyArray;
}

void DatabaseModule::databaseList(QSet<QString> &databaseList) const {
    for (const QString &databaseKey: m_databaseHash.keys()) {
        databaseList.insert(databaseKey);
    }
}

void DatabaseModule::databaseInsert(int index, const QString &key) {
    if (index == -1) index = g_databaseStandardItemModel->rowCount();
    auto *keyItem = new QStandardItem(key); // NOLINT
    keyItem->setData(false, Qt::WhatsThisRole);
    auto *valueItem = new QStandardItem(); // NOLINT
    g_databaseStandardItemModel->insertRow(index, {keyItem, valueItem});
    databaseIndex();
}

void DatabaseModule::databaseRemove(const int index) {
    g_databaseStandardItemModel->removeRow(index);
    databaseIndex();
}

void DatabaseModule::databaseRename(const int index, const QString &key) {
    g_databaseStandardItemModel->item(index, 0)->setText(key);
    databaseIndex();
}

void DatabaseModule::databaseSwap(const int src, const int dst) {
    const auto tmp = g_databaseStandardItemModel->takeRow(src);
    g_databaseStandardItemModel->insertRow(dst, tmp);
    databaseIndex();
    QMetaObject::invokeMethod(m_rootItem, "reload");
}

void DatabaseModule::databaseClear(const int index) {
    if (index == -1) {
        for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
            g_databaseStandardItemModel->item(i, 1)->setText("");
        }
    } else {
        g_databaseStandardItemModel->item(index, 1)->setText("");
    }
}

void DatabaseModule::databaseWrite(const QString &key, const QString &value, bool &status) {
    if (!m_databaseHash.contains(key)) return;
    const auto index = m_databaseHash[key];
    g_databaseStandardItemModel->item(index, 1)->setText(value);
    status = true;
}

void DatabaseModule::databaseIndex() {
    m_databaseHash.clear();
    for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
        const QString key = g_databaseStandardItemModel->item(i, 0)->text();
        m_databaseHash.insert(key, i);
    }
}
