#include "document/page/markdownPage.h"

#include <QShortcut>
#include <QSplitter>

#include "document/module/editorWidget.h"
#include "document/module/scintillaWidget.h"
#include "document/module/webviewWidget.h"
#include "util/uniCast.h"

// public
MarkdownPage::MarkdownPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl)),
      m_webviewWidget(new WebviewWidget()) {
    auto *splitter = new QSplitter(Qt::Horizontal); // NOLINT
    splitter->addWidget(m_editorWidget);
    splitter->addWidget(m_webviewWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({600, 600});
    setWidget(splitter);
    m_webviewWidget->setHtml(uni_cast<QFullHtmlString>(this->handler()->textGet()));
    connect(m_editorWidget, &EditorWidget::appendLog, this, &MarkdownPage::appendLog);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &MarkdownPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &MarkdownPage::changeSelection);
    connect(m_editorWidget, &EditorWidget::changeContent, this, [this] {
        m_webviewWidget->setHtml(uni_cast<QFullHtmlString>(this->handler()->textGet()));
    });
}

void MarkdownPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorWidget->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]},
        {"fileModulePropertyDialog", objects["fileModulePropertyDialog"]},
        {"documentModuleGotoDialog", objects["documentModuleGotoDialog"]}
    });
}

void MarkdownPage::documentSave() {
    m_editorWidget->documentSave();
}

bool MarkdownPage::documentClose(const bool force) {
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

void MarkdownPage::documentGoto() const {
    m_editorWidget->documentGoto();
}

// private
void MarkdownPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
