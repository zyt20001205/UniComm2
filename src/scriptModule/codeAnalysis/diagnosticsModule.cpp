#include "scriptModule/codeAnalysis/diagnosticsModule.h"

#include <QClipboard>
#include <QHeaderView>
#include <QJsonArray>
#include <QQmlContext>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QTimer>

#include "globals.h"
#include "utils/qtUtils.h"

// DiagnosticsModule public
DiagnosticsModule::DiagnosticsModule()
    : DockWidget("Diagnostics"),
      m_diagnosticsWidget(new QQuickWidget()) {
    setWidget(m_diagnosticsWidget);
}

DiagnosticsModule::~DiagnosticsModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] diagnostics module destructed").arg(timestamp);
}

void DiagnosticsModule::propertySet(const QVariantMap &objects) {
    m_diagnosticsWidget->rootContext()->setContextProperty("diagnosticMenu", qvariant_cast<QObject *>(objects["diagnosticsModuleDiagnosticMenu"]));

    const QVariantList horizontalHeader = {"", tr("Source"), tr("Code"), tr("Data"), tr("Message")};
    m_diagnosticsWidget->rootContext()->setContextProperty("diagnosticsModule", this);
    m_diagnosticsWidget->rootContext()->setContextProperty("horizontalHeader", horizontalHeader);
    m_diagnosticsWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_diagnosticsWidget->setSource(QUrl("qrc:/qml/scriptModule/codeAnalysis/diagnosticsModule.qml"));
    m_rootItem = m_diagnosticsWidget->rootObject();
}

void DiagnosticsModule::diagnosticsNotification(const QUrl &scriptUrl, const QJsonArray &diagnostics) {
    QStandardItemModel *diagnosticsModel{};
    if (!m_diagnosticsModelHash.contains(scriptUrl)) {
        diagnosticsModel = new QStandardItemModel(this); // NOLINT
    } else {
        diagnosticsModel = m_diagnosticsModelHash[scriptUrl];
        diagnosticsModel->clear();
    }
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        // range
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const QVariantHash position = {
            {"scriptUrl", scriptUrl},
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
    if (!m_diagnosticsModelHash.contains(scriptUrl)) {
        m_diagnosticsModelHash.insert(scriptUrl, diagnosticsModel);
        QMetaObject::invokeMethod(m_rootItem, "append", Q_ARG(QVariant, scriptUrl.fileName()), Q_ARG(QVariant, QVariant::fromValue(diagnosticsModel)));
    } else {
        m_diagnosticsModelHash[scriptUrl] = diagnosticsModel;
    }
}

void DiagnosticsModule::diagnosticCopy(const QString &diagnostic) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(diagnostic);
}

void DiagnosticsModule::indicatorInsert(const QVariantHash &position) {
    emit setCursorPosition(
        position["scriptUrl"].toUrl(),
        position["startLine"].toInt(),
        position["startCharacter"].toInt());
    emit insertIndicator(
        position["scriptUrl"].toUrl(),
        INDICATOR_SELECTION,
        position["startLine"].toInt(),
        position["startCharacter"].toInt(),
        position["endLine"].toInt(),
        position["endCharacter"].toInt(),
        1000);
}
