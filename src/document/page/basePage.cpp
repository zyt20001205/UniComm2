#include "document/page/basePage.h"

#include <QDir>

#include "globals.h"

// public
BasePage::BasePage(const QUrl &documentUrl)
    : DockWidget(documentUrl.toString()),
      m_documentUrl(documentUrl) {
    setTitle(documentUrl.fileName());
    BasePage::permissionGet();
    emit appendLog(LogLevel::Info, "document opened", QString("<a href='%1'>%2</a>").arg(m_documentUrl.toString(), m_documentUrl.toString()));
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
    documentInfo.isWritable() ? setIcon(QIcon()) : setIcon(QIcon(":/icon/lockClosed.svg"));
}

// protected
void BasePage::closeEvent(QCloseEvent *event) {
    if (!documentClose()) {
        event->ignore();
        return;
    }
    deleteLater();
    emit appendLog(LogLevel::Info, "document closed", QString("<a href='%1'>%2</a>").arg(m_documentUrl.toString(), m_documentUrl.toString()));
    event->accept();
}

bool BasePage::documentClose() {
    return true;
}
