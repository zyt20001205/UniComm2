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

int DatatableModule::datatableInsert(int index, const QString &key) {
    if (index == -1) index = g_datatableHeaderItemModel->rowCount();
    if (index < 0 || index > g_datatableHeaderItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Invalid data table index."));
        return -1;
    }

    auto _key = key.trimmed();
    if (_key.isEmpty()) {
        int suffix{};
        do {
            _key = suffix == 0 ? QStringLiteral("new") : QStringLiteral("new%1").arg(suffix);
            ++suffix;
        } while (m_datatableHash.contains(_key));
    }
    if (m_datatableHash.contains(_key)) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Key \"%1\" already exists.").arg(_key));
        return -1;
    }

    g_undo->push(
        tr("Data Table Insert (%1)").arg(_key),
        [this, index, _key] { _datatableInsert(index, _key); },
        [this, _key] { _datatableRemove(_key); });
    return index;
}

void DatatableModule::datatableRemove(const int index) {
    if (index < 0 || index >= g_datatableHeaderItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Invalid data table index."));
        return;
    }

    const auto key = g_datatableHeaderItemModel->item(index, 0)->text();
    g_undo->push(
        tr("Data Table Remove (%1)").arg(key),
        [this, key] { _datatableRemove(key); },
        [this, index, key] { _datatableInsert(index, key); });
}

bool DatatableModule::datatableRename(const int index, const QString &key) {
    if (index < 0 || index >= g_datatableHeaderItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Invalid data table index."));
        return false;
    }

    const auto _key = key.trimmed();
    if (_key.isEmpty()) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Key cannot be empty."));
        return false;
    }

    const auto oldKey = g_datatableHeaderItemModel->item(index, 0)->text();
    if (_key == oldKey) return true;
    if (m_datatableHash.contains(_key)) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Key \"%1\" already exists.").arg(_key));
        return false;
    }

    g_undo->push(
        tr("Data Table Rename (%1->%2)").arg(oldKey, _key),
        [this, oldKey, newKey = _key] { _datatableRename(oldKey, newKey); },
        [this, oldKey, newKey = _key] { _datatableRename(newKey, oldKey); });
    return true;
}

void DatatableModule::datatableMove(const int src, const int dst) {
    if (src < 0 || src >= g_datatableHeaderItemModel->rowCount() || dst < 0 || dst >= g_datatableHeaderItemModel->rowCount()) {
        m_toast->show(ToastLevel::Warning, tr("Data Table"), tr("Invalid data table index."));
        return;
    }
    if (src == dst) return;

    const auto key = g_datatableHeaderItemModel->item(src, 0)->text();
    g_undo->push(
        tr("Data Table Move (%1: %2->%3)").arg(key).arg(src + 1).arg(dst + 1),
        [this, src, dst] { _datatableMove(src, dst); },
        [this, src, dst] { _datatableMove(dst, src); });
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
    if (!m_datatableHash.contains(key)) return false;
    const auto col = m_datatableHash[key];
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
    const auto iterator = m_datatableHash.constFind(key);
    m_datatableStates.remove(key);
    g_datatableHeaderItemModel->removeRow(iterator.value());
    g_datatableStandardItemModel->removeColumn(iterator.value());
    datatableCache();
}

void DatatableModule::_datatableRename(const QString &oldKey, const QString &newKey) {
    const auto iterator = m_datatableHash.constFind(oldKey);
    m_datatableStates.insert(newKey, m_datatableStates.take(oldKey));
    g_datatableHeaderItemModel->item(iterator.value(), 0)->setText(newKey);
    datatableCache();
}

void DatatableModule::_datatableMove(const int src, const int dst) {
    auto items = g_datatableHeaderItemModel->takeRow(src);
    g_datatableHeaderItemModel->insertRow(dst, items);
    items = g_datatableStandardItemModel->takeColumn(src);
    g_datatableStandardItemModel->insertColumn(dst, items);
    datatableCache();
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
