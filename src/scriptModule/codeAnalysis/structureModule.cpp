#include "scriptModule/codeAnalysis/structureModule.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

#include "globals.h"

// public
StructureModule::StructureModule()
    : DockWidget("Structure"),
      m_structureWidget(new QQuickWidget()),
      m_structureStandardItemModel(new QStandardItemModel()) {
    setWidget(m_structureWidget);
    m_structureWidget->installEventFilter(this);
}

StructureModule::~StructureModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] structure module destructed").arg(timestamp);
}

void StructureModule::propertySet(const QVariantMap &objects) {
    m_structureWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["structureModuleRootMenu"]));
    m_structureWidget->rootContext()->setContextProperty("mainTooltip", qvariant_cast<QObject *>(objects["mainWindowTooltip"]));

    m_structureWidget->rootContext()->setContextProperty("structureModule", this);
    m_structureWidget->rootContext()->setContextProperty("standardItemModel", m_structureStandardItemModel);
    m_structureWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_structureWidget->setSource(QUrl("qrc:/qml/scriptModule/codeAnalysis/structureModule.qml"));
}

void StructureModule::propertyGet(const QVariantMap &objects) {
    m_structureTreeView = qvariant_cast<QObject *>(objects["treeView"]);
}

void StructureModule::documentSymbolResponse(const QUrl &scriptUrl, const QJsonArray &result) {
    m_documentSymbolHash[scriptUrl] = result;
    if (scriptUrl == m_currentScriptUrl) {
        m_structureStandardItemModel->clear();
        documentSymbolPublish(result, nullptr);
    }
}

void StructureModule::scriptFocus(const QUrl &scriptUrl, const QVariantHash &session) {
    if (scriptUrl == m_currentScriptUrl) return;
    m_currentScriptUrl = scriptUrl;
    m_structureStandardItemModel->clear();
    if (m_documentSymbolHash.contains(scriptUrl)) {
        documentSymbolPublish(m_documentSymbolHash[scriptUrl], nullptr);
    }
}

void StructureModule::markerAdd(const QVariantHash &position) {
    emit setFocus(m_currentScriptUrl,
                  true);
    emit setIndex(m_currentScriptUrl,
                  position["startLine"].toInt(),
                  position["startCharacter"].toInt());
    emit addMarker(m_currentScriptUrl,
                   MARKER_HINT,
                   position["startLine"].toInt(),
                   1000);
}

// protected
bool StructureModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_structureWidget) {
        if (event->type() == QEvent::FocusOut) {
            m_structureTreeView->setProperty("selectedRow", -1);
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// private
void StructureModule::documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) const {
    for (const auto &value: result) {
        const auto symbol = value.toObject();
        auto *item = new QStandardItem(); // NOLINT
        const auto kind = symbol["kind"].toInt();
        const auto detail = symbol["detail"].toString();
        const auto name = symbol["name"].toString();
        const auto range = symbol["range"].toObject();
        const auto start = range["start"].toObject();
        const auto end = range["end"].toObject();
        item->setData(name, Qt::DisplayRole);
        item->setData(QVariantHash{
                          {"detail", detail},
                          {
                              "position", QVariantHash{
                                  {"startLine", start["line"].toInt()},
                                  {"startCharacter", start["character"].toInt()},
                                  {"endLine", end["line"].toInt()},
                                  {"endCharacter", end["character"].toInt()}
                              }
                          },
                      }, Qt::WhatsThisRole);
        switch (kind) {
            case SYMBOL_KIND_PACKAGE: {
                item->setData(QUrl("qrc:/icon/symbolPackage.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_METHOD:
            case SYMBOL_KIND_FUNCTION: {
                item->setData(QUrl("qrc:/icon/symbolMethod.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_VARIABLE: {
                item->setData(QUrl("qrc:/icon/symbolVariable.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_CONSTANT: {
                item->setData(QUrl("qrc:/icon/symbolConstant.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_STRING: {
                item->setData(QUrl("qrc:/icon/symbolString.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_NUMBER: {
                item->setData(QUrl("qrc:/icon/symbolNumeric.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_BOOLEAN: {
                item->setData(QUrl("qrc:/icon/symbolBoolean.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_ARRAY: {
                item->setData(QUrl("qrc:/icon/symbolArray.svg"), Qt::DecorationRole);
            }
            break;
            case SYMBOL_KIND_OBJECT: {
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
            }
            break;
            default: {
                item->setData(QUrl("qrc:/icon/symbolMisc.svg"), Qt::DecorationRole);
                qDebug() << "WIP structure kind:" << kind << name << detail;
            }
            break;
        }
        if (parentItem) {
            parentItem->appendRow(item);
        } else {
            m_structureStandardItemModel->appendRow(item);
        }
        if (symbol.contains("children")) {
            QJsonArray children = symbol["children"].toArray();
            documentSymbolPublish(children, item);
        }
    }
}
