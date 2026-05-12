#include "document/page/basePage.h"

#include <QDir>

#include "globals.h"
#include "core/globalManager.h"

// public
BasePage::BasePage(const QUrl &documentUrl)
    : DockWidget(documentUrl.toString()),
      m_documentUrl(documentUrl) {
    setTitle(documentUrl.fileName());
    BasePage::permissionGet();
}

void BasePage::pathDisambiguation() {
    const QString documentPath = m_documentUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QString relatedPath = QDir(workspacePath).relativeFilePath(documentPath);
    setTitle(relatedPath);
}

void BasePage::documentReload() {
}

QUrl BasePage::documentUrl() {
    return m_documentUrl;
}

void BasePage::permissionGet() {
    const QString documentPath = m_documentUrl.toLocalFile();
    const QFileInfo documentInfo(documentPath);
    documentInfo.isWritable() ? setIcon(QIcon()) :
    g_global->themeGet() == Theme::Light ? setIcon(QIcon(":/icon/lockLight.svg")) :
    setIcon(QIcon(":/icon/lockDark.svg"));
}

// protected
void BasePage::closeEvent(QCloseEvent *event) {
    if (!documentClose()) {
        event->ignore();
        return;
    }
    emit closeDocument(m_documentUrl);
    deleteLater();
    event->accept();
}

bool BasePage::documentClose() {
    return true;
}
