#include "document/module/conflictWidget.h"

#include <QDir>
#include <QFile>
#include <QShortcut>
#include <QTimer>

#include "globals.h"
#include "core/globalManager.h"
#include "document/module/scintillaWidget.h"

// public
ConflictWidget::ConflictWidget(const QJsonObject &documentConfig, const QUrl &documentUrl, QWidget *parent)
    : EditorWidget(documentConfig, documentUrl, parent),
      m_contentTimer(new QTimer(this)) {
    connect(m_scintillaWidget, &ScintillaEdit::modified, this, [this](const Scintilla::ModificationFlags type) {
        if (static_cast<int>(type) & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_PERFORMED_UNDO | SC_PERFORMED_REDO)) m_contentTimer->start();
    });
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &ConflictWidget::contentChange);
}

void ConflictWidget::propertySet(const QVariantHash &objects) {
    EditorWidget::propertySet(objects);
    QTimer::singleShot(0, [this] { contentChange(); });
}

// protected
void ConflictWidget::marginInit() const {
    m_scintillaWidget->marginDefine(
        0,
        QVariantHash{
            {"type", SC_MARGIN_NUMBER},
            {"width", 32},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
    m_scintillaWidget->marginDefine(
        1,
        QVariantHash{
            {"type", SC_MARGIN_SYMBOL},
            {"width", 16},
            {"mask", static_cast<int>(~SC_MASK_FOLDERS & ~SC_MASK_HISTORY)},
            {"sensitive", true},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
    m_scintillaWidget->marginDefine(
        2,
        QVariantHash{
            {"type", SC_MARGIN_SYMBOL},
            {"width", 16},
            {"mask", static_cast<int>(SC_MASK_FOLDERS)},
            {"sensitive", true},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
}

void ConflictWidget::markerInit() const {
    EditorWidget::markerInit();
    m_scintillaWidget->markerDefine(
        ScintillaMarker::Hint,
        QVariantHash{
            {"symbol", 22},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
}

// private: slot
void ConflictWidget::marginClick(const Scintilla::Position position, const int mouseButton, const Scintilla::KeyMod modifiers, const int margin) {
    const int line = m_scintillaWidget->lineGet(position);
    if (margin == 1) {
        if (mouseButton == Qt::LeftButton) {
            if (m_scintillaWidget->markerGet(line) & 1 << ScintillaMarker::Navigation) {
            }
        }
    }
}

void ConflictWidget::contentChange() {
    qDebug() << "content changed";
}
