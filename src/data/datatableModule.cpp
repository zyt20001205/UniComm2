#include "data/datatableModule.h"

#include <QDir>
#include <QFile>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTransposeProxyModel>
#include <QVariantList>

#include "globals.h"
#include "core/globalManager.h"
#include "mainWindow/toastModule.h"
#include "util/uniCast.h"

// public
DatatableModule::DatatableModule()
    : DockWidget("Data Table"),
      m_widget(new QQuickWidget()) {
    setWidget(m_widget);
    g_datatableHeaderItemModel = new QStandardItemModel(this);
    g_datatableStandardItemModel = new QStandardItemModel(this);
    m_transposeProxyModel = new QTransposeProxyModel(this);
    m_transposeProxyModel->setSourceModel(g_datatableHeaderItemModel);
}

DatatableModule::~DatatableModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void DatatableModule::propertySet(const QVariantHash &objects) {
    m_toast = qvariant_cast<ToastModule *>(objects["mainWindowToast"]);
    for (const auto &value: g_workspaceConfig["datatableConfig"].toArray()) {
        datatableInsert(-1, value.toString());
    }

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

    const auto sessionHash = QVariantHash{
        {"length", 0}
    };
    m_datatableSession.insert(_key, sessionHash);

    auto *item = new QStandardItem(_key); // NOLINT
    item->setData(false, Qt::WhatsThisRole);
    g_datatableHeaderItemModel->insertRow(index, item);
    g_datatableStandardItemModel->insertColumn(index);
    datatableIndex();
    return index;
}

void DatatableModule::datatableRemove(const int index) {
    const auto key = g_datatableHeaderItemModel->item(index, 0)->text();
    m_datatableSession.remove(key);

    g_datatableHeaderItemModel->removeRow(index);
    g_datatableStandardItemModel->removeColumn(index);
    datatableIndex();
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

    const auto oldSession = m_datatableSession.take(oldKey);
    m_datatableSession.insert(_key, oldSession);

    g_datatableHeaderItemModel->item(index, 0)->setText(_key);
    datatableIndex();
    return true;
}

void DatatableModule::datatableSwap(const int src, const int dst) {
    auto tmp = g_datatableHeaderItemModel->takeRow(src);
    g_datatableHeaderItemModel->insertRow(dst, tmp);
    tmp = g_datatableStandardItemModel->takeColumn(src);
    g_datatableStandardItemModel->insertColumn(dst, tmp);
    datatableIndex();
}

void DatatableModule::datatableClear() {
    for (auto &session: m_datatableSession) {
        session["length"] = 0;
    }
    g_datatableStandardItemModel->clear();
}

void DatatableModule::datatableExport(const QString &path) {
    LPath luaPath = path;
    if (path.isEmpty()) luaPath = "data_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    const auto documentPath = documentUrl.toLocalFile();
    QFile file(documentPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
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
    file.close();
    emit appendLog(LogLevel::Info, "data export to", QString("<a href='%1'>%2</a>").arg(documentUrl.toString(), documentUrl.toString()));
}

bool DatatableModule::datatableWrite(const QString &key, const QString &value) {
    if (!m_datatableHash.contains(key)) return false;
    const auto col = m_datatableHash[key];
    const auto row = m_datatableSession[key]["length"].toInt();
    auto *item = new QStandardItem(value); // NOLINT
    g_datatableStandardItemModel->setItem(row, col, item);
    m_datatableSession[key]["length"] = m_datatableSession[key]["length"].toInt() + 1;
    return true;
}

// private
void DatatableModule::datatableIndex() {
    m_datatableHash.clear();
    for (int i = 0; i < g_datatableHeaderItemModel->rowCount(); ++i) {
        const QString key = g_datatableHeaderItemModel->item(i, 0)->text();
        m_datatableHash.insert(key, i);
    }
}
