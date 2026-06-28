#include "document/page/markdownPage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QShortcut>
#include <QSplitter>

#include "globals.h"
#include "core/globalManager.h"
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
    m_webviewWidget->setHtml(pageGenerate(this->handler()->textGet()));
    connect(m_editorWidget, &EditorWidget::appendLog, this, &MarkdownPage::appendLog);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &MarkdownPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &MarkdownPage::changeSelection);
    connect(m_editorWidget, &EditorWidget::changeContent, this, [this] {
        m_webviewWidget->setHtml(pageGenerate(this->handler()->textGet()));
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

QString MarkdownPage::pageGenerate(const QString &text) {
    auto html = QString(R"(<!doctype html>
<html>
<head>
<meta charset="utf-8">
<style>
html, body {
    margin: 0;
    min-height: 100%;
    color: @fore;
    background: @back;
    font-family: "Segoe UI", sans-serif;
    font-size: 14px;
}
body {
    padding: 12px;
}
a {
    color: @link;
}
pre, code {
    font-family: Consolas, monospace;
}
pre {
    overflow: auto;
    padding: 8px;
    background: @hover;
}
table {
    border-collapse: collapse;
    margin: 8px 0;
}
th, td {
    border: 1px solid @stroke;
    padding: 4px 8px;
}
th {
    background: @hover;
    font-weight: 600;
}
tr:nth-child(even) td {
    background: @selected;
}
img {
    max-width: 100%;
}
.mermaid {
    overflow: auto;
}
.mermaid svg {
    max-width: 100%;
}
</style>
<script src="http://unicomm/mermaid.min.js"></script>
<script>
document.addEventListener("DOMContentLoaded", async () => {
    const blocks = document.querySelectorAll("pre > code.language-mermaid, pre > code.mermaid");
    if (!window.mermaid || blocks.length === 0) return;

    blocks.forEach((code) => {
        const diagram = document.createElement("div");
        diagram.className = "mermaid";
        diagram.textContent = code.textContent;
        code.parentElement.replaceWith(diagram);
    });

    mermaid.initialize({
        startOnLoad: false,
        securityLevel: "loose",
        theme: "@mermaidTheme"
    });
    await mermaid.run({ querySelector: ".mermaid" });
});
</script>
</head>
<body>@body</body>
</html>)");
    html.replace("@fore", g_globalManager->foreGet());
    html.replace("@back", g_globalManager->backGet());
    html.replace("@link", g_globalManager->brandLinkGet());
    html.replace("@hover", g_globalManager->backHoverGet());
    html.replace("@stroke", g_globalManager->strokeGet());
    html.replace("@selected", g_globalManager->backSelectedGet());
    html.replace("@mermaidTheme", g_globalManager->themeGet() == Theme::Light ? "default" : "dark");
    html.replace("@body", uni_cast<QHtmlString>(text));
    return html;
}
