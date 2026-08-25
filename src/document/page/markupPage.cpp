#include "document/page/markupPage.h"

#include <QFileInfo>
#include <QSplitter>

#include "document/module/editorWidget.h"
#include "document/module/scintillaWidget.h"
#include "document/module/webviewWidget.h"
#include "util/uniCast.h"

// public
MarkupPage::MarkupPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : DocumentPage(documentUrl),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl)),
      m_webviewWidget(new WebviewWidget()) {
    auto *splitter = new QSplitter(Qt::Horizontal); // NOLINT
    splitter->addWidget(m_editorWidget);
    splitter->addWidget(m_webviewWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({600, 600});
    setWidget(splitter);
    previewUpdate();
    connect(m_editorWidget, &EditorWidget::appendLog, this, &MarkupPage::appendLog);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &MarkupPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &MarkupPage::changeSelection);
    connect(m_editorWidget, &EditorWidget::changeContent, this, &MarkupPage::previewUpdate);
}

void MarkupPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorWidget->propertySet(QVariantHash{
        {"theme", objects["theme"]},
        {"mainWindowToast", objects["mainWindowToast"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]},
        {"fileModulePropertyDialog", objects["fileModulePropertyDialog"]},
        {"documentModuleGotoDialog", objects["documentModuleGotoDialog"]}
    });
}

void MarkupPage::documentSave() {
    m_editorWidget->documentSave();
}

bool MarkupPage::documentClose(const bool force) {
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
void MarkupPage::previewUpdate() {
    const auto text = handler()->textGet();
    if (QFileInfo(m_documentUrl.toLocalFile()).suffix().toLower() == "md") {
        m_webviewWidget->setHtml(uni_cast<QFullHtmlString>(text));
    } else {
        m_webviewWidget->setHtml(text);
    }
}

void MarkupPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
