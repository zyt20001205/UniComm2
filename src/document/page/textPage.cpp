#include "document/page/textPage.h"

#include <QDir>
#include <QFileInfo>
#include <QShortcut>

#include "globals.h"
#include "document/module/editorWidget.h"
#include "document/module/scintillaWidget.h"

// public
TextPage::TextPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : DocumentPage(documentUrl),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl, this)) {
    setWidget(m_editorWidget);
    connect(m_editorWidget, &EditorWidget::appendLog, this, &TextPage::appendLog);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &TextPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &TextPage::changeSelection);
}

void TextPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorWidget->propertySet(QVariantHash{
        {"theme", objects["theme"]},
        {"mainWindowToast", objects["mainWindowToast"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]},
        {"fileModulePropertyDialog", objects["fileModulePropertyDialog"]},
        {"documentModuleGotoDialog", objects["documentModuleGotoDialog"]}
    });
}

void TextPage::documentSave() {
    m_editorWidget->documentSave();
}

bool TextPage::documentClose(const bool force) {
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

// private
void TextPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
