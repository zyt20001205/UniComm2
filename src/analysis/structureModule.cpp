#include "analysis/structureModule.h"

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
      m_widget(new QQuickWidget()),
      m_standardItemModel(new QStandardItemModel()) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
}

StructureModule::~StructureModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 module destructed").arg(timestamp, uniqueName());
}

void StructureModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["structureModuleRootMenu"]));
    m_widget->rootContext()->setContextProperty("mainToolTip", qvariant_cast<QObject *>(objects["mainWindowToolTip"]));

    m_widget->rootContext()->setContextProperty("structureModule", this);
    m_widget->rootContext()->setContextProperty("standardItemModel", m_standardItemModel);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/analysis/structureModule.qml"));
}

void StructureModule::propertyGet(const QVariantMap &objects) {
    m_treeView = qvariant_cast<QObject *>(objects["treeView"]);
}

void StructureModule::documentSymbolResponse(const QUrl &documentUrl, const QJsonArray &result) {
    m_documentSymbolHash[documentUrl] = result;
    if (documentUrl == m_documentUrl) {
        m_standardItemModel->clear();
        documentSymbolPublish(result, nullptr);
    }
}

void StructureModule::documentFocus(const QUrl &documentUrl, const QVariantHash &session) {
    if (documentUrl == m_documentUrl) return;
    m_documentUrl = documentUrl;
    m_standardItemModel->clear();
    if (m_documentSymbolHash.contains(documentUrl)) {
        documentSymbolPublish(m_documentSymbolHash[documentUrl], nullptr);
    }
}

void StructureModule::markerAdd(const QVariantHash &position) {
    emit setFocus(m_documentUrl,
                  true);
    emit setIndex(m_documentUrl,
                  position["startLine"].toInt(),
                  position["startCharacter"].toInt());
    emit addMarker(m_documentUrl,
                   MARKER_HINT,
                   position["startLine"].toInt(),
                   1000);
}

bool StructureModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget) {
        if (event->type() == QEvent::FocusOut) {
            m_treeView->setProperty("selectedRow", -1);
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// private
void StructureModule::documentSymbolPublish(const QJsonArray &result, QStandardItem *parentItem) {
    for (const auto &value: result) {
        const auto symbol = value.toObject();
        auto *item = new QStandardItem(); // NOLINT
        const auto kind = symbol["kind"].toInt();
        const auto detail = symbol["detail"].toString().trimmed();
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
                emit appendLog(LOG_WARNING, "contact author:", QString("unsupported symbol (kind:%1, name:%2, detail:%3)").arg(QString::number(kind), name, detail));
            }
            break;
        }
        if (parentItem) {
            parentItem->appendRow(item);
        } else {
            m_standardItemModel->appendRow(item);
        }
        if (symbol.contains("children")) {
            QJsonArray children = symbol["children"].toArray();
            documentSymbolPublish(children, item);
        }
    }
}
