#include "dataModule/datatableModule.h"

#include <QDir>
#include <QFile>
#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QVariantList>

#include "globals.h"

DatatableModule::DatatableModule()
    : DockWidget("data table"),
      m_datatableWidget(new QQuickWidget()) {
    setWidget(m_datatableWidget);
    g_datatableStringListModel = new QStringListModel(this);
    g_datatableStandardItemModel = new QStandardItemModel(this);
    for (const auto &value: g_workspaceConfig["datatableConfig"].toArray()) {
        const QString key = value.toString();
        datatableInsert(-1, key);
    }
}

DatatableModule::~DatatableModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] datatable module destructed").arg(timestamp);
}

void DatatableModule::propertySet(const QVariantMap &objects) {
    m_datatableWidget->rootContext()->setContextProperty("nameDialog", qvariant_cast<QObject *>(objects["datatableModuleNameDialog"]));
    m_datatableWidget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["datatableModuleTableMenu"]));
    m_datatableWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["datatableModuleRootMenu"]));

    m_datatableWidget->rootContext()->setContextProperty("datatableModule", this);
    m_datatableWidget->rootContext()->setContextProperty("stringListModel", g_datatableStringListModel);
    m_datatableWidget->rootContext()->setContextProperty("standardItemModel", g_datatableStandardItemModel);
    m_datatableWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_datatableWidget->setSource(QUrl("qrc:/qml/dataModule/datatableModule.qml"));
    m_rootItem = m_datatableWidget->rootObject();
}

void DatatableModule::datatableConfigSave() const {
    QJsonArray keyArray{};
    for (int i = 0; i < g_datatableStringListModel->rowCount(); ++i) {
        const QModelIndex modelIndex = g_datatableStringListModel->index(i);
        const QString key = g_datatableStringListModel->data(modelIndex, Qt::DisplayRole).toString();
        keyArray.append(key);
    }
    g_workspaceConfig["datatableConfig"] = keyArray;
}

void DatatableModule::datatableList(QSet<QString> &datatableList) const {
    for (const QString &datatableKey: m_datatableHash.keys()) {
        datatableList.insert(datatableKey);
    }
}

void DatatableModule::datatableInsert(int index, const QString &key) {
    const auto sessionHash = QVariantHash{
        {"length", 0}
    };
    m_datatableSession.insert(key, sessionHash);

    if (index == -1) index = g_datatableStringListModel->rowCount();
    g_datatableStringListModel->insertRow(index);
    const QModelIndex modelIndex = g_datatableStringListModel->index(index);
    g_datatableStringListModel->setData(modelIndex, key, Qt::DisplayRole);
    g_datatableStringListModel->setData(modelIndex, false, Qt::WhatsThisRole);
    g_datatableStandardItemModel->insertColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRemove(const int index) {
    const QModelIndex modelIndex = g_datatableStringListModel->index(index);
    const auto key = g_datatableStringListModel->data(modelIndex).toString();
    m_datatableSession.remove(key);

    g_datatableStringListModel->removeRow(index);
    g_datatableStandardItemModel->removeColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRename(const int index, const QString &key) {
    const QModelIndex modelIndex = g_datatableStringListModel->index(index);
    const auto oldKey = g_datatableStringListModel->data(modelIndex).toString();
    const auto oldSession = m_datatableSession.take(oldKey);
    m_datatableSession.insert(key, oldSession);

    g_datatableStringListModel->setData(modelIndex, key, Qt::DisplayRole);
    datatableIndex();
}

void DatatableModule::datatableSwap(const int src, const int dst) {
    const QModelIndex srcIndex = g_datatableStringListModel->index(src);
    const auto key = g_datatableStringListModel->data(srcIndex).toString();
    g_datatableStringListModel->insertRow(dst);
    const QModelIndex dstIndex = g_datatableStringListModel->index(dst);
    g_datatableStringListModel->setData(dstIndex, key, Qt::DisplayRole);
    const auto tmp = g_databaseStandardItemModel->takeRow(src);
    g_databaseStandardItemModel->insertRow(dst, tmp);
    datatableIndex();
    QMetaObject::invokeMethod(m_rootItem, "reload");
}

void DatatableModule::datatableClear() {
    for (auto &session: m_datatableSession) {
        session["length"] = 0;
    }
    g_datatableStandardItemModel->clear();
}

void DatatableModule::datatableExport() {
    const auto workspacePath = g_workspaceUrl.toLocalFile();
    const QString defaultName = "data_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
    const auto filePath = QDir(workspacePath).filePath(defaultName);
    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    // write header
    QStringList keyList{};
    for (int i = 0; i < g_datatableStringListModel->rowCount(); ++i) {
        const QModelIndex modelIndex = g_datatableStringListModel->index(i);
        const QString key = g_datatableStringListModel->data(modelIndex, Qt::DisplayRole).toString();
        keyList.append(key);
    }
    const QString header = keyList.join(", ") + "\n";
    out << header;
    // write data
    for (int i = 0; i < g_datatableStandardItemModel->rowCount(); ++i) {
        QStringList rowData{};
        for (int j = 0; j < g_datatableStandardItemModel->columnCount(); ++j) {
            rowData.append(g_datatableStandardItemModel->item(i, j)->text());
        }
        out << rowData.join(",") << "\n";
    }
    file.close();
    // logging
    const QUrl fileUrl = QUrl::fromLocalFile(filePath);
    emit appendLog(QString("data exported to <a href='%1'>%2</a>").arg(fileUrl.toString(), fileUrl.toString()), "info");
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] data exported").arg(timestamp);
}

void DatatableModule::datatableWrite(const QString &key, const QString &value, bool &status) {
    if (!m_datatableHash.contains(key)) return;
    const auto col = m_datatableHash[key];
    const auto row = m_datatableSession[key]["length"].toInt();
    auto *item = new QStandardItem(value); // NOLINT
    g_datatableStandardItemModel->setItem(row, col, item);
    m_datatableSession[key]["length"] = m_datatableSession[key]["length"].toInt() + 1;
    status = true;
}

// DatatableModule private
void DatatableModule::datatableIndex() {
    m_datatableHash.clear();
    for (int i = 0; i < g_datatableStringListModel->rowCount(); ++i) {
        const QModelIndex modelIndex = g_datatableStringListModel->index(i);
        const QString key = g_datatableStringListModel->data(modelIndex, Qt::DisplayRole).toString();
        m_datatableHash.insert(key, i);
    }
}
