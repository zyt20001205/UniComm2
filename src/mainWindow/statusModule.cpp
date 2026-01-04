#include "mainWindow/statusModule.h"

#include <QDir>
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
    setResizeMode(SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/mainWindow/statusModule.qml"));
    m_rootItem = rootObject();
}

void StatusModule::propertyGet(const QVariantMap &objects) {
}

void StatusModule::scriptFocus(const QUrl &scriptUrl) const {
    const QString scriptPath = scriptUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QDir workspaceDir(workspacePath);
    const QString relativePath = workspaceDir.relativeFilePath(scriptPath);
    const QVariantList pathList = QVariant::fromValue(relativePath.split('/')).toList();
    QMetaObject::invokeMethod(m_rootItem, "scriptPathLoad", Q_ARG(QVariant, QVariant::fromValue(pathList)));
}
