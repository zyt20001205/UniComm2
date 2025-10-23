#include "scriptModule/structureModule.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

#include "globals.h"

// StructureModule public
StructureModule::StructureModule()
    : DockWidget("structure"),
      m_documentSymbolTreeView(new QTreeView()),
      m_documentSymbolTreeModel(new QStandardItemModel()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->addWidget(m_documentSymbolTreeView);
    layout->setContentsMargins(0, 0, 0, 0);
    m_documentSymbolTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_documentSymbolTreeView->setHeaderHidden(true);
    m_documentSymbolTreeView->setModel(m_documentSymbolTreeModel);
    connect(m_documentSymbolTreeView, &QTreeView::clicked, this, [this](const QModelIndex &index) {
        const int line = index.data(Qt::UserRole + 1).toInt() + 1;
        emit showMarker(m_currentScriptUrl, MARKER_HINT, line, 1000);
    });
}

void StructureModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        // qDebug() << result;
        m_documentSymbolTreeModel->clear();
        documentSymbolPublish(result, nullptr);
        m_documentSymbolTreeView->expandAll();
    }
}

void StructureModule::scriptFocus(const QUrl &scriptUrl) {
    m_currentScriptUrl = scriptUrl;
    if (m_documentSymbolHash.contains(scriptUrl)) {
        m_documentSymbolTreeModel->clear();
        documentSymbolPublish(m_documentSymbolHash[scriptUrl], nullptr);
        m_documentSymbolTreeView->expandAll();
    }
}

// StructureModule private
void StructureModule::documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const {
    for (const auto &value: result) {
        const auto symbolObject = value.toObject();
        QString displayText{};
        int line = 0;
        switch (symbolObject["kind"].toInt()) {
            case SYMBOLKIND_FUNCTION: {
                const auto detail = symbolObject["detail"].toString();
                const auto name = symbolObject["name"].toString();
                displayText = name + detail.mid(9);
                const auto rangeObject = symbolObject["range"].toObject();
                const auto startObject = rangeObject["start"].toObject();
                line = startObject["line"].toInt();
            }
            break;
            default: break;
        }
        if (!displayText.isEmpty()) {
            auto *item = new QStandardItem(displayText); // NOLINT
            item->setData(line, Qt::UserRole + 1);
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
