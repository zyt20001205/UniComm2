#include "mainWindow/statusModule.h"

#include <QDir>
#include <QQmlContext>
#include <QQuickItem>

#include "globals.h"

// StatusModule public
StatusModule::StatusModule(QWidget *parent)
    : QQuickWidget(parent) {
}

StatusModule::~StatusModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] status module destructed").arg(timestamp);
}

void StatusModule::propertySet(const QVariantMap &objects) {
    rootContext()->setContextProperty("statusModule", this);
    rootContext()->setContextProperty("workspaceName", g_workspaceUrl.fileName());
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/mainWindow/statusModule.qml"));
    m_rootItem = rootObject();
}

void StatusModule::propertyGet(const QVariantMap &objects) {
    m_positionButton = qvariant_cast<QObject *>(objects["positionButton"]);
    m_threadButton = qvariant_cast<QObject *>(objects["threadButton"]);
}

void StatusModule::scriptFocus(const QUrl &scriptUrl) const {
    const QString scriptPath = scriptUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QDir workspaceDir(workspacePath);
    const QString relativePath = workspaceDir.relativeFilePath(scriptPath);
    const QVariantList pathList = QVariant::fromValue(relativePath.split('/')).toList();
    QMetaObject::invokeMethod(m_rootItem, "scriptPathLoad", Q_ARG(QVariant, QVariant::fromValue(pathList)));
}

void StatusModule::selectionChange(const QHash<QString, int> &selection) const {
    const auto current = QString("%1:%2").arg(QString::number(selection["line"] + 1), QString::number(selection["character"]));
    const auto charsText = selection["characters"] == 0 ? "" : QString(" [%1 chars]").arg(QString::number(selection["characters"]));
    const auto linesText = selection["lines"] == 0 ? "" : QString(" [%1 lines]").arg(QString::number(selection["lines"]));
    m_positionButton->setProperty("text", QString("%1%2%3").arg(current, charsText, linesText));
}

void StatusModule::threadRefresh(const int run, const int debug) const {
    if (run + debug == 0) {
        m_threadButton->setProperty("text", tr("Idle"));
    } else {
        m_threadButton->setProperty("text", QString(tr("Run: %1 Debug: %2")).arg(QString::number(run), QString::number(debug)));
    }
}
