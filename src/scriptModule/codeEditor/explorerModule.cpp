#include "scriptModule/codeEditor/explorerModule.h"

#include <QFileSystemModel>
#include <QInputDialog>
#include <QMenu>
#include <QProcess>
#include <QQmlContext>
#include <QTreeView>

#include "globals.h"

// ExplorerModule public
ExplorerModule::ExplorerModule()
    : DockWidget("explorer"),
      m_explorerWidget(new QQuickWidget()),
      m_explorerFileSystemModel(new QFileSystemModel()),
      m_explorerTreeView(new QTreeView()) {
    setWidget(m_explorerWidget);
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

void ExplorerModule::scriptRun(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    QString threadId{};
    emit startThread(scriptUrl, LUATHREAD_RUN, threadId);
}

void ExplorerModule::scriptDebug(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath);
    QFile file(scriptPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString script = in.readAll();
    file.close();
    QString threadId{};
    emit startThread(scriptUrl, LUATHREAD_DEBUG, threadId);
}

void ExplorerModule::scriptOpen(const QString &scriptPath) {
    const QUrl scriptUrl = QUrl::fromLocalFile(scriptPath).toString();
    emit openScript(scriptUrl);
}
