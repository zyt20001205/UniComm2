#include "data/databaseModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"

// public
DatabaseModule::DatabaseModule()
    : DockWidget("Database"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
    g_databaseStandardItemModel = new QStandardItemModel(this);
    for (const auto &value: g_workspaceConfig["databaseConfig"].toArray()) {
        const QString key = value.toString();
        databaseInsert(-1, key);
    }
}

DatabaseModule::~DatabaseModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] database module destructed").arg(timestamp);
}

void DatabaseModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("editDialog", qvariant_cast<QObject *>(objects["databaseModuleEditDialog"]));
    m_widget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["databaseModuleTableMenu"]));
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["databaseModuleRootMenu"]));

    m_widget->rootContext()->setContextProperty("databaseModule", this);
    m_widget->rootContext()->setContextProperty("standardItemModel", g_databaseStandardItemModel);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/data/databaseModule.qml"));
    m_root = m_widget->rootObject();
}

void DatabaseModule::databaseConfigSave() {
    QJsonArray keys{};
    for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
        const QString key = g_databaseStandardItemModel->item(i, 0)->text();
        keys.append(key);
    }
    g_workspaceConfig["databaseConfig"] = keys;
}

QSet<QString> DatabaseModule::databaseList() const {
    QSet<QString> keys{};
    for (const auto &databaseKey: m_databaseHash.keys()) {
        keys.insert(databaseKey);
    }
    return keys;
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
    QMetaObject::invokeMethod(m_root, "reload");
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

bool DatabaseModule::databaseWrite(const QString &key, const QString &value) {
    if (!m_databaseHash.contains(key)) return false;
    const auto index = m_databaseHash[key];
    g_databaseStandardItemModel->item(index, 1)->setText(value);
    return true;
}

// private
void DatabaseModule::databaseIndex() {
    m_databaseHash.clear();
    for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
        const QString key = g_databaseStandardItemModel->item(i, 0)->text();
        m_databaseHash.insert(key, i);
    }
}
