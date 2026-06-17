#include "document/page/conflictPage.h"

#include <QDir>
#include <QFileInfo>
#include <QShortcut>

#include "globals.h"
#include "document/module/editorWidget.h"
#include "document/module/scintillaWidget.h"

// public
ConflictPage::ConflictPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl, this)) {
    setWidget(m_editorWidget);
    connect(m_editorWidget, &EditorWidget::appendLog, this, &ConflictPage::appendLog);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &ConflictPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &ConflictPage::changeSelection);
}

void ConflictPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorWidget->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]},
        {"fileModulePropertyDialog", objects["fileModulePropertyDialog"]},
        {"documentModuleGotoDialog", objects["documentModuleGotoDialog"]}
    });
}

void ConflictPage::documentSave() {
    m_editorWidget->documentSave();
}

bool ConflictPage::documentClose(const bool force) {
    if (force) {
        emit closeDocument(m_documentUrl);
        deleteLater();
        return true;
    }
    bool status = true;
    if (handler()->modifyGet()) {
        m_saveDialog->setProperty("documentUrl", m_documentUrl);
        m_saveDialog->setProperty("documentName", m_documentUrl.fileName());
        QMetaObject::invokeMethod(m_saveDialog, "open");
        const auto eventloop = new QEventLoop(this);
        const auto conn = connect(m_saveDialog, SIGNAL(closed()), eventloop, SLOT(quit()));
        eventloop->exec();
        disconnect(conn);
        delete eventloop;
        status = m_saveDialog->property("status").toBool();
    }
    return status;
}

void ConflictPage::documentGoto() const {
    m_editorWidget->documentGoto();
}

// private
void ConflictPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
