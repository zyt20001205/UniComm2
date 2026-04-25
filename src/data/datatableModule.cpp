#include "data/datatableModule.h"

#include <QDir>
#include <QFile>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QVariantList>

#include "globals.h"
#include "util/uniCast.h"

// public
DatatableModule::DatatableModule()
    : DockWidget("Data Table"),
      m_datatableWidget(new QQuickWidget()) {
    setWidget(m_datatableWidget);
    g_datatableHeaderItemModel = new QStandardItemModel(this);
    g_datatableStandardItemModel = new QStandardItemModel(this);
    for (const auto &value: g_workspaceConfig["datatableConfig"].toArray()) {
        const QString key = value.toString();
        datatableInsert(-1, key);
    }
}

DatatableModule::~DatatableModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] datatable module destructed").arg(timestamp);
}

void DatatableModule::propertySet(const QVariantMap &objects) {
    m_datatableWidget->rootContext()->setContextProperty("editDialog", qvariant_cast<QObject *>(objects["datatableModuleEditDialog"]));
    m_datatableWidget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["datatableModuleTableMenu"]));
    m_datatableWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["datatableModuleRootMenu"]));

    m_datatableWidget->rootContext()->setContextProperty("datatableModule", this);
    m_datatableWidget->rootContext()->setContextProperty("headerItemModel", g_datatableHeaderItemModel);
    m_datatableWidget->rootContext()->setContextProperty("standardItemModel", g_datatableStandardItemModel);
    m_datatableWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_datatableWidget->setSource(QUrl("qrc:/qml/data/datatableModule.qml"));
    m_rootItem = m_datatableWidget->rootObject();
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

void DatatableModule::datatableInsert(int index, const QString &key) {
    const auto sessionHash = QVariantHash{
        {"length", 0}
    };
    m_datatableSession.insert(key, sessionHash);

    if (index == -1) index = g_datatableHeaderItemModel->rowCount();
    auto *item = new QStandardItem(key); // NOLINT
    item->setData(false, Qt::WhatsThisRole);
    g_datatableHeaderItemModel->insertRow(index, item);
    g_datatableStandardItemModel->insertColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRemove(const int index) {
    const auto key = g_datatableHeaderItemModel->item(index, 0)->text();
    m_datatableSession.remove(key);

    g_datatableHeaderItemModel->removeRow(index);
    g_datatableStandardItemModel->removeColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRename(const int index, const QString &key) {
    const auto oldKey = g_datatableHeaderItemModel->item(index, 0)->text();
    const auto oldSession = m_datatableSession.take(oldKey);
    m_datatableSession.insert(key, oldSession);

    g_datatableHeaderItemModel->item(index, 0)->setText(key);
    datatableIndex();
}

void DatatableModule::datatableSwap(const int src, const int dst) {
    auto tmp = g_datatableHeaderItemModel->takeRow(src);
    g_datatableHeaderItemModel->insertRow(dst, tmp);
    tmp = g_datatableStandardItemModel->takeColumn(src);
    g_datatableStandardItemModel->insertColumn(dst, tmp);
    datatableIndex();
    QMetaObject::invokeMethod(m_rootItem, "reload");
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
    emit appendLog(LOG_INFO, "data export to", QString("<a href='%1'>%2</a>").arg(documentUrl.toString(), documentUrl.toString()));
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
