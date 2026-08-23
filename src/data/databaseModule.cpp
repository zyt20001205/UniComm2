#include "data/databaseModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"
#include "core/globalManager.h"
#include "mainWindow/toastModule.h"

// public
DatabaseModule::DatabaseModule()
    : DockWidget("Database"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
    g_databaseStandardItemModel = new DatabaseModel();
}

DatabaseModule::~DatabaseModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void DatabaseModule::propertySet(const QVariantHash &objects) {
    m_toast = qvariant_cast<ToastModule *>(objects["mainWindowToast"]);
    for (const auto &value: g_workspaceConfig["databaseConfig"].toArray()) {
        databaseInsert(-1, value.toString());
    }

    m_widget->rootContext()->setContextProperty("databaseModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["databaseModuleTableMenu"]));
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["databaseModuleRootMenu"]));
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

int DatabaseModule::databaseInsert(int index, const QString &key) {
    if (index == -1) index = g_databaseStandardItemModel->rowCount();
    if (index < 0 || index > g_databaseStandardItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Invalid database index."), 3000);
        return -1;
    }

    auto _key = key.trimmed();
    if (_key.isEmpty()) {
        int suffix{};
        do {
            _key = suffix == 0 ? QStringLiteral("new") : QStringLiteral("new%1").arg(suffix);
            ++suffix;
        } while (m_databaseHash.contains(_key));
    }
    if (m_databaseHash.contains(_key)) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Key \"%1\" already exists.").arg(_key), 3000);
        return -1;
    }
    auto *keyItem = new QStandardItem(_key); // NOLINT
    keyItem->setData(_key, DatabaseModel::KeyRole);
    auto *valueItem = new QStandardItem(); // NOLINT
    g_databaseStandardItemModel->insertRow(index, {keyItem, valueItem});
    databaseIndex();
    return index;
}

void DatabaseModule::databaseRemove(const int index) {
    g_databaseStandardItemModel->removeRow(index);
    databaseIndex();
}

bool DatabaseModule::databaseRename(const int index, const QString &key) {
    if (index < 0 || index >= g_databaseStandardItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Invalid database index."), 3000);
        return false;
    }

    const auto _key = key.trimmed();
    if (_key.isEmpty()) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Key cannot be empty."), 3000);
        return false;
    }

    auto *item = g_databaseStandardItemModel->item(index, 0);
    if (_key == item->text()) return true;
    if (m_databaseHash.contains(_key)) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Key \"%1\" already exists.").arg(_key), 3000);
        return false;
    }

    item->setText(_key);
    item->setData(_key, DatabaseModel::KeyRole);
    databaseIndex();
    return true;
}

void DatabaseModule::databaseSwap(const int src, const int dst) {
    const auto tmp = g_databaseStandardItemModel->takeRow(src);
    g_databaseStandardItemModel->insertRow(dst, tmp);
    databaseIndex();
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

// public
DatabaseModel::DatabaseModel(QObject *parent)
    : QStandardItemModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &DatabaseModel::emptyChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &DatabaseModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &DatabaseModel::emptyChanged);
}

QHash<int, QByteArray> DatabaseModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[KeyRole] = "key";
    return roles;
}

QVariant DatabaseModel::data(const QModelIndex &index, const int role) const {
    if (role == KeyRole) {
        return QStandardItemModel::data(this->index(index.row(), 0), role);
    }
    return QStandardItemModel::data(index, role);
}
