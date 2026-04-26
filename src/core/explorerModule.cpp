#include "core/explorerModule.h"

#include <QFileSystemModel>
#include <QQmlContext>
#include <QQuickWidget>

#include "globals.h"

// public
ExplorerModule::ExplorerModule()
    : DockWidget("Explorer"),
      m_widget(new QQuickWidget()),
      m_fileSystemModel(new QFileSystemModel()) {
    setWidget(m_widget);
    m_widget->installEventFilter(this);
}

ExplorerModule::~ExplorerModule() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] explorer module destructed").arg(timestamp);
}

void ExplorerModule::propertySet(const QVariantMap &objects) {
    m_widget->rootContext()->setContextProperty("scriptMenu", qvariant_cast<QObject *>(objects["explorerModuleFileMenu"]));
    m_widget->rootContext()->setContextProperty("folderMenu", qvariant_cast<QObject *>(objects["explorerModuleFolderMenu"]));
    m_widget->rootContext()->setContextProperty("rootMenu", qvariant_cast<QObject *>(objects["explorerModuleRootMenu"]));

    const auto modelRootPath = g_workspaceUrl.toLocalFile();
    m_fileSystemModel->setRootPath(modelRootPath);
    const QModelIndex modelRootIndex = m_fileSystemModel->index(modelRootPath);
    m_widget->rootContext()->setContextProperty("explorerModule", this);
    m_widget->rootContext()->setContextProperty("modelRootIndex", modelRootIndex);
    m_widget->rootContext()->setContextProperty("modelRootPath", modelRootPath);
    m_widget->rootContext()->setContextProperty("fileSystemModel", m_fileSystemModel);
    m_widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_widget->setSource(QUrl("qrc:/qml/core/explorerModule.qml"));
}

void ExplorerModule::propertyGet(const QVariantMap &objects) {
    m_treeView = qvariant_cast<QObject *>(objects["treeView"]);
}

void ExplorerModule::scriptRun(const QString &documentPath) {
    const QUrl documentUrl = QUrl::fromLocalFile(documentPath);
    emit startThread(documentUrl, THREAD_RUN, -1, -1, -1, -1);
}

void ExplorerModule::scriptDebug(const QString &documentPath) {
    const QUrl documentUrl = QUrl::fromLocalFile(documentPath);
    emit startThread(documentUrl, THREAD_DEBUG, -1, -1, -1, -1);
}

void ExplorerModule::documentOpen(const QString &documentPath) {
    const QUrl documentUrl = QUrl::fromLocalFile(documentPath).toString();
    emit openDocument(documentUrl);
}

bool ExplorerModule::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_widget) {
        if (event->type() == QEvent::FocusOut) {
            m_treeView->setProperty("selectedRow", -1);
        }
    }
    return DockWidget::eventFilter(watched, event);
}
