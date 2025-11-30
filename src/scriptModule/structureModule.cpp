#include "scriptModule/structureModule.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

#include "globals.h"

// StructureModule public
StructureModule::StructureModule()
    : DockWidget("structure"),
      m_structureWidget(new QQuickWidget()),
      m_documentSymbolAbstractModel(new QStandardItemModel()) {
    setWidget(m_structureWidget);
    m_structureWidget->rootContext()->setContextProperty("structureModule", this);
    m_structureWidget->rootContext()->setContextProperty("filterModel", m_documentSymbolAbstractModel);
    m_structureWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_structureWidget->setSource(QUrl("qrc:/qml/scriptModule/structureModule.qml"));
}

void StructureModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        m_documentSymbolAbstractModel->clear();
        documentSymbolPublish(result, nullptr);
    }
}

void StructureModule::scriptFocus(const QUrl &scriptUrl) {
    if (m_currentScriptUrl == scriptUrl) return;
    m_currentScriptUrl = scriptUrl;
    m_documentSymbolAbstractModel->clear();
    if (m_documentSymbolHash.contains(scriptUrl)) {
        documentSymbolPublish(m_documentSymbolHash[scriptUrl], nullptr);
    }
}

void StructureModule::markerInsert(const int row) {
    // const int line = m_documentSymbolAbstractModel->item(row,0)->data(Qt::UserRole + 1).toInt();
    // emit insertMarker(m_currentScriptUrl, MARKER_HINT, line, 1000);
}

// StructureModule private
void StructureModule::documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const {
    for (const auto &value: result) {
        const auto symbol = value.toObject();
        auto *item = new QStandardItem(); // NOLINT
        const auto kind = symbol["kind"].toInt();
        const auto detail = symbol["detail"].toString();
        const auto name = symbol["name"].toString();
        const auto range = symbol["range"].toObject();
        const auto start = range["start"].toObject();
        const int line = start["line"].toInt();
        switch (kind) {
            case SYMBOLKIND_FUNCTION: {
                item->setData(name + detail.mid(9), Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolMethod.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
            }
            break;
            case SYMBOLKIND_NUMBER: {
                item->setData(name + " = " + detail, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolNumeric.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
            }
            break;
            case SYMBOLKIND_CONSTANT: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolConstant.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
            }
            break;
            case SYMBOLKIND_STRING: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolString.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
            }
            break;
            case SYMBOLKIND_OBJECT: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
            }
            break;
            default: {
                item->setText(name);
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
                item->setData(line, Qt::UserRole + 1);
                qDebug() << "WIP structure kind:" << kind << name << detail;
            }
            break;
        }
        if (parentItem) {
            parentItem->appendRow(item);
        } else {
            m_documentSymbolAbstractModel->appendRow(item);
        }
        if (symbol.contains("children")) {
            QJsonArray children = symbol["children"].toArray();
            documentSymbolPublish(children, item);
        }
    }
}
