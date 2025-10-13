#include "scriptModule/structureModule.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

// StructureModule public
StructureModule::StructureModule(QWidget *parent)
    : QDockWidget("structure", parent),
      m_documentSymbolTreeView(new QTreeView()),
      m_documentSymbolTreeModel(new QStandardItemModel()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->addWidget(m_documentSymbolTreeView);
    layout->setContentsMargins(0, 0, 0, 0);
    m_documentSymbolTreeView->setModel(m_documentSymbolTreeModel);
    m_documentSymbolTreeView->setHeaderHidden(true);
}

void StructureModule::documentSymbolReturn(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        qDebug() << result;
        m_documentSymbolTreeModel->clear();
        documentSymbolPublish(result, nullptr);
        m_documentSymbolTreeView->expandAll();
    }
}

void StructureModule::scriptSwitch(const QUrl &scriptUrl) {
    m_currentScriptUrl = scriptUrl;
}

// StructureModule private
void StructureModule::documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const {
    for (const auto &value: result) {
        const auto symbolObject = value.toObject();
        QString displayText{};
        switch (symbolObject["kind"].toInt()) {
            case SYMBOLKIND_FUNCTION: {
                const auto detail = symbolObject["detail"].toString();
                const auto name = symbolObject["name"].toString();
                displayText = name + detail.mid(9);
            }
            break;
            default: break;
        }
        if (!displayText.isEmpty()) {
            auto *item = new QStandardItem(displayText); // NOLINT
            if (parentItem) {
                parentItem->appendRow(item);
            } else {
                m_documentSymbolTreeModel->appendRow(item);
            }
            if (symbolObject.contains("children")) {
                QJsonArray children = symbolObject["children"].toArray();
                documentSymbolPublish(children, item);
            }
        }
    }
}
