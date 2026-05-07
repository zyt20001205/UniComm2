#include "document/module/editorWidget.h"

#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

#include "globals.h"
#include "core/globalManager.h"
#include "document/module/scintillaWidget.h"
#include "document/module/searchWidget.h"

// public
EditorWidget::EditorWidget(const QVariantHash& session, QWidget* parent)
    : QWidget(parent),
      m_session(session),
      m_scintillaWidget(new ScintillaWidget(this)),
      m_searchWidget(new SearchWidget(this)) {
    auto* layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_scintillaWidget);
    layout->addWidget(m_searchWidget);
    miscInit();
    textInit();
}

void EditorWidget::propertySet(const QVariantHash& objects) const {
    m_searchWidget->propertySet(QVariantHash{
        {"global", objects["global"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
}

// protected
void EditorWidget::miscInit() const {
    // annotation
    m_scintillaWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
    m_scintillaWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
    // folding
    m_scintillaWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1")); // NOLINT
    m_scintillaWidget->send(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE); // NOLINT
    m_scintillaWidget->send(SCI_SETFOLDMARGINCOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
    m_scintillaWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
    m_scintillaWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
    m_scintillaWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
    // scrollbar
    m_scintillaWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
    m_scintillaWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT
    // selection
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, ScintillaWidget::colorGet(g_global->brandBackGet(), 128)); // NOLINT
    m_scintillaWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET, ScintillaWidget::colorGet(g_global->foreGet(), 255)); // NOLINT
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, ScintillaWidget::colorGet(g_global->backSelectedGet(), 128)); // NOLINT
    m_scintillaWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
    // tab
    m_scintillaWidget->send(SCI_SETUSETABS, false); // NOLINT
    m_scintillaWidget->send(SCI_SETINDENT, 4); // NOLINT
    m_scintillaWidget->send(SCI_SETTABINDENTS, true); // NOLINT
    m_scintillaWidget->send(SCI_SETBACKSPACEUNINDENTS, true); // NOLINT
    m_scintillaWidget->send(SCI_SETINDENTATIONGUIDES, SC_IV_REAL); // NOLINT
}

void EditorWidget::textInit() {
    // text load
    const auto documentPath = m_session["documentUrl"].toUrl().toLocalFile();
    auto documentFile = QFile(documentPath);
    if (!documentFile.open(QIODevice::ReadOnly)) return;
    auto documentTextStream = QTextStream(&documentFile);
    const auto documentText = documentTextStream.readAll();
    documentFile.close();
    m_scintillaWidget->textSet(documentText);
    // permission check
    const auto documentInfo = QFileInfo(documentPath);
    m_scintillaWidget->readonlySet(!documentInfo.isWritable());
    // history record
    m_scintillaWidget->send(SCI_EMPTYUNDOBUFFER); // NOLINT
    m_scintillaWidget->send(SCI_SETCHANGEHISTORY,SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS); // NOLINT
}
