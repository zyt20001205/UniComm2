#include "document/page/textPage.h"

#include <QDir>
#include <QFileInfo>
#include <QShortcut>

#include "globals.h"
#include "document/module/editorWidget.h"

// public
TextPage::TextPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_editorWidget(new EditorWidget(documentConfig, documentUrl, this)){
    setWidget(m_editorWidget);
    connect(m_editorWidget, &EditorWidget::changeSavepoint, this, &TextPage::savepointChange);
    connect(m_editorWidget, &EditorWidget::changeSelection, this, &TextPage::changeSelection);
}

void TextPage::propertySet(const QVariantHash &objects) const {
    m_editorWidget->propertySet(QVariantHash{
            {"global", objects["global"]},
            {"mainWindowToolTip", objects["mainWindowToolTip"]},
            {"fileModulePropertyDialog", objects["fileModulePropertyDialog"]}
    });
}

void TextPage::documentSave() const {
    m_editorWidget->documentSave();
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
