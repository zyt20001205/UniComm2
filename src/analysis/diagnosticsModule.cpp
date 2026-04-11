#include "analysis/diagnosticsModule.h"

#include <QClipboard>
#include <QHeaderView>
#include <QJsonArray>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardItemModel>
#include <QTimer>

#include "globals.h"
#include "util/qtUtils.h"

// public
DiagnosticsModule::DiagnosticsModule()
    : DockWidget("Diagnostics"),
      m_diagnosticsWidget(new QQuickWidget()) {
    setWidget(m_diagnosticsWidget);
}

DiagnosticsModule::~DiagnosticsModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] diagnostics module destructed").arg(timestamp);
}

void DiagnosticsModule::propertySet(const QVariantMap &objects) {
    m_diagnosticsWidget->rootContext()->setContextProperty("diagnosticMenu", qvariant_cast<QObject *>(objects["diagnosticsModuleDiagnosticMenu"]));

    const QVariantList horizontalHeader = {"", tr("Source"), tr("Code"), tr("Data"), tr("Message")};
    m_diagnosticsWidget->rootContext()->setContextProperty("diagnosticsModule", this);
    m_diagnosticsWidget->rootContext()->setContextProperty("horizontalHeader", horizontalHeader);
    m_diagnosticsWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_diagnosticsWidget->setSource(QUrl("qrc:/qml/analysis/diagnosticsModule.qml"));
    m_rootItem = m_diagnosticsWidget->rootObject();
}

void DiagnosticsModule::diagnosticsNotification(const QUrl &documentUrl, const QJsonArray &diagnostics) {
    QStandardItemModel *diagnosticsModel{};
    if (!m_diagnosticsModelHash.contains(documentUrl)) {
        diagnosticsModel = new QStandardItemModel(this); // NOLINT
    } else {
        diagnosticsModel = m_diagnosticsModelHash[documentUrl];
        diagnosticsModel->clear();
    }
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        // range
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const QVariantHash position = {
            {"documentUrl", documentUrl},
            {"startLine", start["line"].toInt()},
            {"startCharacter", start["character"].toInt()},
            {"endLine", end["line"].toInt()},
            {"endCharacter", end["character"].toInt()}
        };
        // information
        const int severity = diagnostic["severity"].toInt();
        const QString source = diagnostic["source"].toString();
        const QString code = diagnostic["code"].toString();
        const QString data = diagnostic["data"].toString();
        // placeholder operation
        if (diagnostic["message"].toString().contains("__PLACEHOLDER__")) continue;
        const QString message = md2plain(diagnostic["message"].toString());
        auto *severityItem = new QStandardItem(); // NOLINT
        switch (severity) {
            case LEVEL_ERROR: {
                severityItem->setData(QUrl("qrc:/icon/error.svg"), Qt::DecorationRole);
            }
            break;
            case LEVEL_WARNING: {
                severityItem->setData(QUrl("qrc:/icon/warning.svg"), Qt::DecorationRole);
            }
            break;
            case LEVEL_INFO: {
                severityItem->setData(QUrl("qrc:/icon/info.svg"), Qt::DecorationRole);
            }
            break;
            case LEVEL_HINT: {
                severityItem->setData(QUrl("qrc:/icon/hint.svg"), Qt::DecorationRole);
            }
            break;
            default: break;
        }
        severityItem->setData(position, Qt::WhatsThisRole);
        auto *sourceItem = new QStandardItem(source); // NOLINT
        auto *codeItem = new QStandardItem(code); // NOLINT
        auto *dataItem = new QStandardItem(data); // NOLINT
        auto *messageItem = new QStandardItem(message); // NOLINT
        diagnosticsModel->appendRow({severityItem, sourceItem, codeItem, dataItem, messageItem});
    }
    if (!m_diagnosticsModelHash.contains(documentUrl)) {
        m_diagnosticsModelHash.insert(documentUrl, diagnosticsModel);
        QMetaObject::invokeMethod(m_rootItem, "append", Q_ARG(QVariant, documentUrl.fileName()), Q_ARG(QVariant, QVariant::fromValue(diagnosticsModel)));
    } else {
        m_diagnosticsModelHash[documentUrl] = diagnosticsModel;
    }
}

void DiagnosticsModule::diagnosticCopy(const QString &diagnostic) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(diagnostic);
}

void DiagnosticsModule::indicatorFill(const QVariantHash &position) {
    emit setIndex(
        position["documentUrl"].toUrl(),
        position["startLine"].toInt(),
        position["startCharacter"].toInt());
    emit fillIndicator(
        position["documentUrl"].toUrl(),
        INDICATOR_SELECTION,
        position["startLine"].toInt(),
        position["startCharacter"].toInt(),
        position["endLine"].toInt(),
        position["endCharacter"].toInt(),
        1000);
}
