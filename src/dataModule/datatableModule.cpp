#include "dataModule/datatableModule.h"

#include <QQmlContext>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QVariantList>

#include "globals.h"

DatatableModule::DatatableModule()
    : DockWidget("data table"),
      m_datatableWidget(new QQuickWidget()),
      m_datatableStandardItemModel(new QStandardItemModel(this)) {
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
    // m_datatableWidget->rootContext()->setContextProperty("nameDialog", qvariant_cast<QObject *>(objects["datatableModuleNameDialog"]));
    // m_datatableWidget->rootContext()->setContextProperty("tableMenu", qvariant_cast<QObject *>(objects["datatableModuleTableMenu"]));
    // m_datatableWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["datatableModuleRootMenu"]));

    m_datatableWidget->rootContext()->setContextProperty("datatableModule", this);
    m_datatableWidget->rootContext()->setContextProperty("horizontalHeader", g_datatableStringListModel);
    m_datatableWidget->rootContext()->setContextProperty("standardItemModel", m_datatableStandardItemModel);
    m_datatableWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_datatableWidget->setSource(QUrl("qrc:/qml/dataModule/datatableModule.qml"));
    m_rootItem = m_datatableWidget->rootObject();
}

void DatatableModule::datatableConfigSave() const {
}

void DatatableModule::datatableList(QSet<QString> &datatableList) const {
    for (const QString &datatableKey: m_datatableHash.keys()) {
        datatableList.insert(datatableKey);
    }
}

void DatatableModule::datatableInsert(int index, const QString &key) {
    if (index == -1) index = g_datatableStringListModel->rowCount();
    g_datatableStringListModel->insertRow(index);
    const QModelIndex modelIndex = g_datatableStringListModel->index(index);
    g_datatableStringListModel->setData(modelIndex,key,  Qt::DisplayRole);
    m_datatableStandardItemModel->insertColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRemove(const int index) {
    g_datatableStringListModel->removeRow(index);
    m_datatableStandardItemModel->removeColumn(index);
    datatableIndex();
}

void DatatableModule::datatableRename(int index, const QString &key) {
}

void DatatableModule::datatableSwap(int src, int dst) {
}

void DatatableModule::datatableClear(int index) {
}

void DatatableModule::datatableWrite(const QString &key, const QString &value, bool &status) {
}

// void DatatableModule::datatableExport() {
//     const QString defaultName = "data_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv";
//     QFile file(defaultName);
//     file.open(QIODevice::WriteOnly | QIODevice::Text);
//     QTextStream out(&file);
//     // write header
//     const QList<QString> keyList = m_data.keys();
//     const QString header = keyList.join(", ") + "\n";
//     out << header;
//     // calc length
//     int rowCount = 0;
//     foreach(const QString &key, keyList) {
//         rowCount = qMax(rowCount, m_data[key].y.size());
//     }
//     // write data(y)
//     for (int row = 0; row < rowCount; ++row) {
//         QStringList rowData;
//         foreach(const QString &key, keyList) {
//             if (row < m_data[key].y.size()) {
//                 rowData << QString::number(m_data[key].y[row]);
//             } else {
//                 rowData << "";
//             }
//         }
//         out << rowData.join(",") << "\n";
//     }
//     file.close();
//     // logging
//     const QUrl fileUrl = QUrl::fromLocalFile(file.fileName());
//     emit appendLog(QString("data exported to <a href='%1'>%2</a>").arg(fileUrl.toString(), defaultName), "info");
//     const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
//     qDebug() << QString("[%1] data exported").arg(timestamp);
// }

// DatatableModule private
void DatatableModule::datatableIndex() {
    m_datatableHash.clear();
    for (int i = 0; i < g_datatableStringListModel->rowCount(); ++i) {
        const QModelIndex modelIndex = g_datatableStringListModel->index(i);
        const QString key = g_datatableStringListModel->data(modelIndex, Qt::DisplayRole).toString();
        m_datatableHash.insert(key, i);
    }
}
