#include "data/datatableModule.h"

#include <QDir>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QSaveFile>

#include "globals.h"
#include "core/globalManager.h"
#include "core/undoModule.h"
#include "mainWindow/toastModule.h"
#include "util/uniCast.h"

// public
DatatableModule::DatatableModule()
    : DockWidget("Data Table"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
    g_datatableHeaderItemModel = new QStandardItemModel(this);
    g_datatableStandardItemModel = new QStandardItemModel(this);
    m_transposeProxyModel = new DatatableHeaderModel(this);
    m_transposeProxyModel->setSourceModel(g_datatableHeaderItemModel);
    for (const auto &value: g_workspaceConfig["datatableConfig"].toArray()) {
        const auto key = value.toString().trimmed();
        if (key.isEmpty() || m_datatableHash.contains(key)) continue;
        _datatableInsert(g_datatableHeaderItemModel->rowCount(), key);
    }
}

DatatableModule::~DatatableModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void DatatableModule::propertySet(const QVariantHash &objects) {
    m_toast = qvariant_cast<ToastModule *>(objects["mainWindowToast"]);

    m_widget->rootContext()->setContextProperty("datatableModule", this);
    m_widget->rootContext()->setContextProperty("global", g_globalManager);
    m_widget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["datatableModuleTableMenu"]));
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["datatableModuleRootMenu"]));
    m_widget->rootContext()->setContextProperty("headerItemModel", m_transposeProxyModel);
    m_widget->rootContext()->setContextProperty("standardItemModel", g_datatableStandardItemModel);

    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/data/datatableModule.qml"));
    m_item = m_widget->rootObject();
}

void DatatableModule::datatableConfigSave() {
    QJsonArray keyArray{};
    for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
        const QString key = g_datatableHeaderItemModel->item(i, 0)->text();
        keyArray.append(key);
    }
    g_workspaceConfig["datatableConfig"] = keyArray;
}

QSet<QString> DatatableModule::datatableList() const {
    QSet<QString> keys{};
    for (const auto &datatableKey: m_datatableHash.keys()) {
        keys.insert(datatableKey);
    }
    return keys;
}

QString DatatableModule::datatableInsert(const QString &key, const QString &targetKey, const QString &undoGroupId) {
    auto _key = key.trimmed();
    if (_key.isEmpty()) {
        int suffix{};
        do {
            _key = suffix == 0 ? QStringLiteral("new") : QStringLiteral("new%1").arg(suffix);
            ++suffix;
        } while (m_datatableHash.contains(_key));
    }
    if (m_datatableHash.contains(_key)) {
        const auto error = tr("Data table insert failed: key \"%1\" already exists.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }

    int targetIndex = g_datatableHeaderItemModel->rowCount();
    const auto _targetKey = targetKey.trimmed();
    if (!_targetKey.isEmpty()) {
        targetIndex = datatableIndex(_targetKey);
        if (targetIndex == -1) {
            const auto error = tr("Data table insert failed: target key \"%1\" does not exist.").arg(_targetKey);
            if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
            return error;
        }
    }

    const auto index = QSharedPointer<int>::create(targetIndex);
    const auto error = g_undo->push(
        tr("Data Table Insert (%1)").arg(_key),
        [this, index, _key] {
            if (m_datatableHash.contains(_key)) return tr("Data table insert failed: key \"%1\" already exists.").arg(_key);
            if (*index < 0 || *index > g_datatableHeaderItemModel->rowCount()) return tr("Data table insert failed: invalid index.");
            _datatableInsert(*index, _key);
            return QString{};
        },
        [this, index, _key] {
            *index = datatableIndex(_key);
            if (*index == -1) return tr("Data table insert undo failed: key \"%1\" does not exist.").arg(_key);
            _datatableRemove(_key);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
    return error;
}

QString DatatableModule::datatableRemove(const QString &key, const QString &undoGroupId) {
    const auto _key = key.trimmed();
    const auto currentIndex = datatableIndex(_key);
    if (currentIndex == -1) {
        const auto error = tr("Data table remove failed: key \"%1\" does not exist.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }

    const auto index = QSharedPointer<int>::create(currentIndex);
    const auto error = g_undo->push(
        tr("Data Table Remove (%1)").arg(_key),
        [this, index, _key] {
            *index = datatableIndex(_key);
            if (*index == -1) return tr("Data table remove failed: key \"%1\" does not exist.").arg(_key);
            _datatableRemove(_key);
            return QString{};
        },
        [this, index, _key] {
            if (m_datatableHash.contains(_key)) return tr("Data table remove undo failed: key \"%1\" already exists.").arg(_key);
            if (*index < 0 || *index > g_datatableHeaderItemModel->rowCount()) return tr("Data table remove undo failed: invalid index.");
            _datatableInsert(*index, _key);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
    return error;
}

QString DatatableModule::datatableRename(const QString &key, const QString &newKey, const QString &undoGroupId) {
    const auto oldKey = key.trimmed();
    const auto _key = newKey.trimmed();
    if (!m_datatableHash.contains(oldKey)) {
        const auto error = tr("Data table rename failed: key \"%1\" does not exist.").arg(oldKey);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }
    if (_key.isEmpty()) {
        const auto error = tr("Data table rename failed: key cannot be empty.");
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }
    if (_key == oldKey) return {};
    if (m_datatableHash.contains(_key)) {
        const auto error = tr("Data table rename failed: key \"%1\" already exists.").arg(_key);
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }

    const auto error = g_undo->push(
        tr("Data Table Rename (%1->%2)").arg(oldKey, _key),
        [this, oldKey, newKey = _key] {
            if (!m_datatableHash.contains(oldKey)) return tr("Data table rename failed: key \"%1\" does not exist.").arg(oldKey);
            if (m_datatableHash.contains(newKey)) return tr("Data table rename failed: key \"%1\" already exists.").arg(newKey);
            _datatableRename(oldKey, newKey);
            return QString{};
        },
        [this, oldKey, newKey = _key] {
            if (!m_datatableHash.contains(newKey)) return tr("Data table rename undo failed: key \"%1\" does not exist.").arg(newKey);
            if (m_datatableHash.contains(oldKey)) return tr("Data table rename undo failed: key \"%1\" already exists.").arg(oldKey);
            _datatableRename(newKey, oldKey);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
    return error;
}

QString DatatableModule::datatableMove(const QString &key, const QString &targetKey, const QString &undoGroupId) {
    const auto _key = key.trimmed();
    const auto _targetKey = targetKey.trimmed();
    const auto src = datatableIndex(_key);
    const auto dst = datatableIndex(_targetKey);
    if (src == -1 || dst == -1) {
        const auto error = tr("Data table move failed: key does not exist.");
        if (undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
        return error;
    }
    if (src == dst) return {};

    const auto error = g_undo->push(
        tr("Data Table Move (%1: %2->%3)").arg(_key).arg(src + 1).arg(dst + 1),
        [this, _key, _targetKey] {
            const auto currentIndex = datatableIndex(_key);
            const auto targetIndex = datatableIndex(_targetKey);
            if (currentIndex == -1 || targetIndex == -1) return tr("Data table move failed: key does not exist.");
            _datatableMove(currentIndex, targetIndex);
            return QString{};
        },
        [this, src, _key] {
            const auto currentIndex = datatableIndex(_key);
            if (currentIndex == -1) return tr("Data table move undo failed: key \"%1\" does not exist.").arg(_key);
            if (src < 0 || src >= g_datatableHeaderItemModel->rowCount()) return tr("Data table move undo failed: invalid target index.");
            _datatableMove(currentIndex, src);
            return QString{};
        },
        undoGroupId);
    if (!error.isEmpty() && undoGroupId.isEmpty()) m_toast->show(ToastLevel::Warning, tr("Data Table"), error);
    return error;
}

void DatatableModule::datatableClear() {
    for (auto &state: m_datatableStates) {
        state.length = 0;
    }
    g_datatableStandardItemModel->clear();
}

void DatatableModule::datatableExport(const QString &path) {
    LPath luaPath = path;
    if (path.isEmpty()) luaPath = "data_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    const auto documentPath = documentUrl.toLocalFile();
    bool saved = false;
    QSaveFile file(documentPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // write header
        QStringList keyList{};
        for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
            const QString key = g_datatableHeaderItemModel->item(i, 0)->text();
            keyList.append(key);
        }
        const QString header = keyList.join(", ") + "\n";
        out << header;
        // write data
        for (int i = 0; i < g_datatableStandardItemModel->rowCount(); ++i) {
            QStringList rowData{};
            for (int j = 0; j < g_datatableStandardItemModel->columnCount(); ++j) {
                if (g_datatableStandardItemModel->item(i, j)) {
                    rowData.append(g_datatableStandardItemModel->item(i, j)->text());
                } else {
                    rowData.append("");
                }
            }
            out << rowData.join(",") << "\n";
        }
        out.flush();
        if (out.status() == QTextStream::Ok) saved = file.commit();
    }
    if (saved) {
        m_toast->show(ToastLevel::Success, tr("Data Table exported"), documentPath, {
            {tr("Open"), [this, documentUrl] { emit openFileInApplication(documentUrl); }},
            {tr("Show in Explorer"), [this, documentUrl] { emit openFileInExplorer(documentUrl); }}
        });
        emit appendLog(LogLevel::Info, "data export to", QString("<a href='%1'>%2</a>").arg(documentUrl.toString(), documentUrl.toString()));
    } else {
        m_toast->show(ToastLevel::Error, tr("Data Table export failed"), documentPath);
    }
}

bool DatatableModule::datatableWrite(const QString &key, const QString &value) {
    const auto col = datatableIndex(key);
    if (col == -1) return false;
    auto &state = m_datatableStates[key];
    auto *item = new QStandardItem(value); // NOLINT
    g_datatableStandardItemModel->setItem(state.length, col, item);
    ++state.length;
    return true;
}

// private
void DatatableModule::_datatableInsert(const int index, const QString &key) {
    m_datatableStates.insert(key, {});

    auto *item = new QStandardItem(key); // NOLINT
    item->setData(false, Qt::WhatsThisRole);
    g_datatableHeaderItemModel->insertRow(index, item);
    g_datatableStandardItemModel->insertColumn(index);
    datatableCache();
}

void DatatableModule::_datatableRemove(const QString &key) {
    const auto index = datatableIndex(key);
    m_datatableStates.remove(key);
    g_datatableHeaderItemModel->removeRow(index);
    g_datatableStandardItemModel->removeColumn(index);
    datatableCache();
}

void DatatableModule::_datatableRename(const QString &oldKey, const QString &newKey) {
    const auto index = datatableIndex(oldKey);
    m_datatableStates.insert(newKey, m_datatableStates.take(oldKey));
    g_datatableHeaderItemModel->item(index, 0)->setText(newKey);
    datatableCache();
}

void DatatableModule::_datatableMove(const int src, const int dst) {
    auto items = g_datatableHeaderItemModel->takeRow(src);
    g_datatableHeaderItemModel->insertRow(dst, items);
    items = g_datatableStandardItemModel->takeColumn(src);
    g_datatableStandardItemModel->insertColumn(dst, items);
    datatableCache();
}

int DatatableModule::datatableIndex(const QString &key) const {
    return m_datatableHash.value(key, -1);
}

void DatatableModule::datatableCache() {
    m_datatableHash.clear();
    for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
        const QString key = g_datatableHeaderItemModel->item(i, 0)->text();
        m_datatableHash.insert(key, i);
    }
}

// public
DatatableHeaderModel::DatatableHeaderModel(QObject *parent)
    : QTransposeProxyModel(parent) {
    connect(this, &QAbstractItemModel::columnsInserted, this, &DatatableHeaderModel::emptyChanged);
    connect(this, &QAbstractItemModel::columnsRemoved, this, &DatatableHeaderModel::emptyChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &DatatableHeaderModel::emptyChanged);
}
