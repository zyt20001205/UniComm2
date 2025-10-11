#include "scriptModule/structureModule.h"

#include <QJsonArray>
#include <QTreeView>
#include <QVBoxLayout>

// StructureModule public
StructureModule::StructureModule(QWidget *parent)
    : QDockWidget("structure", parent),
      m_documentSymbolTreeView(new QTreeView()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->addWidget(m_documentSymbolTreeView);
    layout->setContentsMargins(0, 0, 0, 0);
}

void StructureModule::documentSymbolReturn(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        qDebug() << result;
    }
}

void StructureModule::scriptSwitch(const QUrl &scriptUrl) {
    m_currentScriptUrl = scriptUrl;
}
