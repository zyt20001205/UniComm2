#include "scriptModule/codeEditor/explorerModule.h"

#include <QFileSystemModel>
#include <QQmlContext>

#include "globals.h"

// public
ExplorerModule::ExplorerModule()
    : DockWidget("Explorer"),
      m_explorerWidget(new QQuickWidget()),
      m_explorerFileSystemModel(new QFileSystemModel()) {
    setWidget(m_explorerWidget);
    m_explorerWidget->installEventFilter(this);
}

ExplorerModule::~ExplorerModule() {
    const QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] explorer module destructed").arg(timestamp);
}

void ExplorerModule::propertySet(const QVariantMap &objects) {
    m_explorerWidget->rootContext()->setContextProperty("scriptMenu", qvariant_cast<QObject *>(objects["explorerModuleScriptMenu"]));
    m_explorerWidget->rootContext()->setContextProperty("folderMenu", qvariant_cast<QObject *>(objects["explorerModuleFolderMenu"]));
    m_explorerWidget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["explorerModuleRootMenu"]));

    const auto modelRootPath = g_workspaceUrl.toLocalFile();
    m_explorerFileSystemModel->setRootPath(modelRootPath);
    const QModelIndex modelRootIndex = m_explorerFileSystemModel->index(modelRootPath);
    m_explorerWidget->rootContext()->setContextProperty("explorerModule", this);
    m_explorerWidget->rootContext()->setContextProperty("modelRootIndex", modelRootIndex);
    m_explorerWidget->rootContext()->setContextProperty("modelRootPath", modelRootPath);
    m_explorerWidget->rootContext()->setContextProperty("fileSystemModel", m_explorerFileSystemModel);
    m_explorerWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_explorerWidget->setSource(QUrl("qrc:/qml/scriptModule/codeEditor/explorerModule.qml"));
}

void ExplorerModule::propertyGet(const QVariantMap &objects) {
    m_explorerTreeView = qvariant_cast<QObject *>(objects["treeView"]);
}

void ExplorerModule::scriptRun(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    emit startThread(scriptUrl, LUATHREAD_RUN, -1, -1, -1, -1);
}

void ExplorerModule::scriptDebug(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    emit startThread(scriptUrl, LUATHREAD_DEBUG, -1, -1, -1, -1);
}

void ExplorerModule::scriptOpen(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath).toString();
    emit openScript(scriptUrl);
}

// protected
bool ExplorerModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_explorerWidget) {
        if (event->type() == QEvent::FocusOut) {
            m_explorerTreeView->setProperty("selectedRow", -1);
        }
    }
    return DockWidget::eventFilter(watched, event);
}
