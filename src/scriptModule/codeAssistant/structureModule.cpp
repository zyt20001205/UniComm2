#include "scriptModule/codeAssistant/structureModule.h"

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
      m_structureStandardModel(new QStandardItemModel()) {
    setWidget(m_structureWidget);
    m_structureWidget->rootContext()->setContextProperty("structureModule", this);
    m_structureWidget->rootContext()->setContextProperty("standardModel", m_structureStandardModel);
    m_structureWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_structureWidget->setSource(QUrl("qrc:/qml/scriptModule/codeAssistant/structureModule.qml"));
}

void StructureModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        m_structureStandardModel->clear();
        documentSymbolPublish(result, nullptr);
    }
}

void StructureModule::scriptFocus(const QUrl &scriptUrl) {
    if (scriptUrl == m_currentScriptUrl) return;
    m_currentScriptUrl = scriptUrl;
    m_structureStandardModel->clear();
    if (m_documentSymbolHash.contains(scriptUrl)) {
        documentSymbolPublish(m_documentSymbolHash[scriptUrl], nullptr);
    }
}

void StructureModule::markerInsert(const int row) {
    emit insertMarker(m_currentScriptUrl, MARKER_HINT, row, 1000);
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
            case SYMBOLKIND_PACKAGE: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolPackage.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
                break;
            case SYMBOLKIND_FUNCTION: {
                item->setData(name + detail.mid(9), Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolMethod.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            case SYMBOLKIND_VARIABLE: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolVariable.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
                break;
            case SYMBOLKIND_CONSTANT: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolConstant.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            case SYMBOLKIND_STRING: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolString.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            case SYMBOLKIND_NUMBER: {
                item->setData(name + " = " + detail, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolNumeric.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            case SYMBOLKIND_ARRAY: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolArray.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            case SYMBOLKIND_OBJECT: {
                item->setData(name, Qt::DisplayRole);
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
            }
            break;
            default: {
                item->setText(name);
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
                item->setData(line, Qt::WhatsThisRole);
                qDebug() << "WIP structure kind:" << kind << name << detail;
            }
            break;
        }
        if (parentItem) {
            parentItem->appendRow(item);
        } else {
            m_structureStandardModel->appendRow(item);
        }
        if (symbol.contains("children")) {
            QJsonArray children = symbol["children"].toArray();
            documentSymbolPublish(children, item);
        }
    }
}
