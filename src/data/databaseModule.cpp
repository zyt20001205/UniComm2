#include "data/databaseModule.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>

#include "globals.h"
#include "core/globalManager.h"
#include "core/undoModule.h"
#include "mainWindow/toastModule.h"

// public
DatabaseModule::DatabaseModule()
    : DockWidget("Database"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
    g_databaseStandardItemModel = new DatabaseModel(this);
    for (const auto &value: g_workspaceConfig["databaseConfig"].toArray()) {
        const auto key = value.toString().trimmed();
        if (key.isEmpty() || m_databaseHash.contains(key)) continue;
        _databaseInsert(g_databaseStandardItemModel->rowCount(), key);
    }
}

DatabaseModule::~DatabaseModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void DatabaseModule::propertySet(const QVariantHash &objects) {
    m_toast = qvariant_cast<ToastModule *>(objects["mainWindowToast"]);

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

QString DatabaseModule::databaseInsert(const QString &key, const QString &targetKey, const QString &undoGroupId) {
    auto _key = key.trimmed();
    if (_key.isEmpty()) {
        int suffix{};
        do {
            _key = suffix == 0 ? QStringLiteral("new") : QStringLiteral("new%1").arg(suffix);
            ++suffix;
        } while (m_databaseHash.contains(_key));
    }
    if (m_databaseHash.contains(_key)) {
        const auto error = tr("Database insert failed: key \"%1\" already exists.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }

    int targetIndex = g_databaseStandardItemModel->rowCount();
    const auto _targetKey = targetKey.trimmed();
    if (!_targetKey.isEmpty()) {
        targetIndex = databaseIndex(_targetKey);
        if (targetIndex == -1) {
            const auto error = tr("Database insert failed: target key \"%1\" does not exist.").arg(_targetKey);
            if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
            return error;
        }
    }

    const auto index = QSharedPointer<int>::create(targetIndex);
    const auto error = g_undo->push(
        tr("Database Insert (%1)").arg(_key),
        [this, index, _key] {
            if (m_databaseHash.contains(_key)) return tr("Database insert failed: key \"%1\" already exists.").arg(_key);
            if (*index < 0 || *index > g_databaseStandardItemModel->rowCount()) return tr("Database insert failed: invalid index.");
            _databaseInsert(*index, _key);
            return QString{};
        },
        [this, index, _key] {
            *index = databaseIndex(_key);
            if (*index == -1) return tr("Database insert undo failed: key \"%1\" does not exist.").arg(_key);
            _databaseRemove(_key);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
    return error;
}

QString DatabaseModule::databaseRemove(const QString &key, const QString &undoGroupId) {
    const auto _key = key.trimmed();
    const auto currentIndex = databaseIndex(_key);
    if (currentIndex == -1) {
        const auto error = tr("Database remove failed: key \"%1\" does not exist.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }

    const auto index = QSharedPointer<int>::create(currentIndex);
    const auto error = g_undo->push(
        tr("Database Remove (%1)").arg(_key),
        [this, index, _key] {
            *index = databaseIndex(_key);
            if (*index == -1) return tr("Database remove failed: key \"%1\" does not exist.").arg(_key);
            _databaseRemove(_key);
            return QString{};
        },
        [this, index, _key] {
            if (m_databaseHash.contains(_key)) return tr("Database remove undo failed: key \"%1\" already exists.").arg(_key);
            if (*index < 0 || *index > g_databaseStandardItemModel->rowCount()) return tr("Database remove undo failed: invalid index.");
            _databaseInsert(*index, _key);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
    return error;
}

QString DatabaseModule::databaseRename(const QString &key, const QString &newKey, const QString &undoGroupId) {
    const auto oldKey = key.trimmed();
    const auto _key = newKey.trimmed();
    if (!m_databaseHash.contains(oldKey)) {
        const auto error = tr("Database rename failed: key \"%1\" does not exist.").arg(oldKey);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }
    if (_key.isEmpty()) {
        const auto error = tr("Database rename failed: key cannot be empty.");
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }
    if (_key == oldKey) return {};
    if (m_databaseHash.contains(_key)) {
        const auto error = tr("Database rename failed: key \"%1\" already exists.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }

    const auto error = g_undo->push(
        tr("Database Rename (%1->%2)").arg(oldKey, _key),
        [this, oldKey, newKey = _key] {
            if (!m_databaseHash.contains(oldKey)) return tr("Database rename failed: key \"%1\" does not exist.").arg(oldKey);
            if (m_databaseHash.contains(newKey)) return tr("Database rename failed: key \"%1\" already exists.").arg(newKey);
            _databaseRename(oldKey, newKey);
            return QString{};
        },
        [this, oldKey, newKey = _key] {
            if (!m_databaseHash.contains(newKey)) return tr("Database rename undo failed: key \"%1\" does not exist.").arg(newKey);
            if (m_databaseHash.contains(oldKey)) return tr("Database rename undo failed: key \"%1\" already exists.").arg(oldKey);
            _databaseRename(newKey, oldKey);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
    return error;
}

QString DatabaseModule::databaseMove(const QString &key, const QString &targetKey, const QString &undoGroupId) {
    const auto _key = key.trimmed();
    const auto _targetKey = targetKey.trimmed();
    const auto src = databaseIndex(_key);
    const auto dst = databaseIndex(_targetKey);
    if (src == -1 || dst == -1) {
        const auto error = tr("Database move failed: key does not exist.");
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
        return error;
    }
    if (src == dst) return {};

    const auto error = g_undo->push(
        tr("Database Move (%1: %2->%3)").arg(_key).arg(src + 1).arg(dst + 1),
        [this, _key, _targetKey] {
            const auto currentIndex = databaseIndex(_key);
            const auto targetIndex = databaseIndex(_targetKey);
            if (currentIndex == -1 || targetIndex == -1) return tr("Database move failed: key does not exist.");
            _databaseMove(currentIndex, targetIndex);
            return QString{};
        },
        [this, src, _key] {
            const auto currentIndex = databaseIndex(_key);
            if (currentIndex == -1) return tr("Database move undo failed: key \"%1\" does not exist.").arg(_key);
            if (src < 0 || src >= g_databaseStandardItemModel->rowCount()) return tr("Database move undo failed: invalid target index.");
            _databaseMove(currentIndex, src);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Database"), error);
    return error;
}

void DatabaseModule::databaseClear(const QString &key) {
    const auto _key = key.trimmed();
    if (_key.isEmpty()) {
        for (int i = 0; i < g_databaseStandardItemModel->rowCount(); ++i) {
            g_databaseStandardItemModel->item(i, 1)->setText("");
        }
        return;
    }

    const auto index = databaseIndex(_key);
    if (index == -1) {
        m_toast->show(ToastLevel::Warning, tr("Database"), tr("Database clear failed: key \"%1\" does not exist.").arg(_key));
        return;
    }
    g_databaseStandardItemModel->item(index, 1)->setText("");
}

bool DatabaseModule::databaseWrite(const QString &key, const QString &value) {
    const auto index = databaseIndex(key);
    if (index == -1) return false;
    g_databaseStandardItemModel->item(index, 1)->setText(value);
    return true;
}

// private
void DatabaseModule::_databaseInsert(const int index, const QString &key) {
    auto *keyItem = new QStandardItem(key); // NOLINT
    auto *valueItem = new QStandardItem(); // NOLINT
    g_databaseStandardItemModel->insertRow(index, {keyItem, valueItem});
    databaseCache();
}

void DatabaseModule::_databaseRemove(const QString &key) {
    g_databaseStandardItemModel->removeRow(databaseIndex(key));
    databaseCache();
}

void DatabaseModule::_databaseRename(const QString &oldKey, const QString &newKey) {
    auto *item = g_databaseStandardItemModel->item(databaseIndex(oldKey), 0);
    item->setText(newKey);
    databaseCache();
}

void DatabaseModule::_databaseMove(const int src, const int dst) {
    const auto row = g_databaseStandardItemModel->takeRow(src);
    g_databaseStandardItemModel->insertRow(dst, row);
    databaseCache();
}

int DatabaseModule::databaseIndex(const QString &key) const {
    return m_databaseHash.value(key, -1);
}

void DatabaseModule::databaseCache() {
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
