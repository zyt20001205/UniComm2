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
    m_scintillaWidget->viewport()->installEventFilter(this);
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &ConflictWidget::contentChange);
}

void ConflictWidget::propertySet(const QVariantHash &objects) {
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    EditorWidget::propertySet(objects);
    QTimer::singleShot(0, [this] { contentChange(); });
}

bool ConflictWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_scintillaWidget->viewport()) {
        const QPoint globalPos = QCursor::pos();
        const QPoint localPos = m_scintillaWidget->viewport()->mapFromGlobal(globalPos);
        if (event->type() == QEvent::MouseButtonPress) {
            const auto *mouseEvent = static_cast<QMouseEvent *>(event);
            const auto &position = m_scintillaWidget->positionGet(localPos);
            // margin click
            if (localPos.x() < m_scintillaWidget->marginWidthGet()) return false;
            // text area click
            if (mouseEvent->button() == Qt::LeftButton) {
                const auto &index = m_scintillaWidget->indexGet(position);
                const auto &line = index["line"];
                if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictStart) {
                    const auto &start = m_hunk.value(line)[0];
                    const auto &separator = m_hunk.value(line)[1];
                    const auto &end = m_hunk.value(line)[2];
                    const auto &text = m_scintillaWidget->textGet(start + 1, 0, separator - 1, -1);
                    m_scintillaWidget->selectionSet(start, 0, end, -1);
                    m_scintillaWidget->textSetSelected(text);
                } else if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictSeparator) {
                    const auto &start = m_hunk.value(line)[0];
                    const auto &separator = m_hunk.value(line)[1];
                    const auto &end = m_hunk.value(line)[2];
                    const auto &text = m_scintillaWidget->textGet(start + 1, 0, separator, 0)
                                       + m_scintillaWidget->textGet(separator + 1, 0, end - 1, -1);
                    m_scintillaWidget->selectionSet(start, 0, end, -1);
                    m_scintillaWidget->textSetSelected(text);
                } else if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictEnd) {
                    const auto &start = m_hunk.value(line)[0];
                    const auto &separator = m_hunk.value(line)[1];
                    const auto &end = m_hunk.value(line)[2];
                    const auto &text = m_scintillaWidget->textGet(separator + 1, 0, end - 1, -1);
                    m_scintillaWidget->selectionSet(start, 0, end, -1);
                    m_scintillaWidget->textSetSelected(text);
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            const auto &position = m_scintillaWidget->positionGet(localPos);
            QString tooltip{};
            if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictStart) tooltip = tr("Accept Current");
            else if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictSeparator) tooltip = tr("Accept Both");
            else if (m_scintillaWidget->indicatorGet(position) & 1 << ScintillaIndicator::ConflictEnd) tooltip = tr("Accept Incoming");
            else tooltip = QString();
            if (!tooltip.isEmpty()) m_toolTip->setProperty("position", QCursor::pos());
            m_toolTip->setProperty("text", tooltip);
            return false;
        }
    }
    return EditorWidget::eventFilter(watched, event);
}

// protected
void ConflictWidget::indicatorInit() const {
    EditorWidget::indicatorInit();
    m_scintillaWidget->indicatorDefine(
        ScintillaIndicator::ConflictStart,
        QVariantHash{
            {"style", INDIC_TEXTFORE},
            {"fore", ScintillaWidget::colorGet(g_globalManager->successFore2Get())},
            {"alpha", 255},
            {"outlineAlpha", 255},
            {"setUnder", false}
        });
    m_scintillaWidget->indicatorDefine(
        ScintillaIndicator::ConflictSeparator,
        QVariantHash{
            {"style", INDIC_TEXTFORE},
            {"fore", ScintillaWidget::colorGet(g_globalManager->brandBackGet())},
            {"alpha", 255},
            {"outlineAlpha", 255},
            {"setUnder", false}
        });
    m_scintillaWidget->indicatorDefine(
        ScintillaIndicator::ConflictEnd,
        QVariantHash{
            {"style", INDIC_TEXTFORE},
            {"fore", ScintillaWidget::colorGet(g_globalManager->warningFore2Get())},
            {"alpha", 255},
            {"outlineAlpha", 255},
            {"setUnder", false}
        });
}

void ConflictWidget::markerInit() const {
    EditorWidget::markerInit();
    m_scintillaWidget->markerDefine(
        ScintillaMarker::ConflictCurrent,
        QVariantHash{
            {"symbol", SC_MARK_BACKGROUND},
            {"back", ScintillaWidget::colorGet(g_globalManager->successBack2Get())}
        });
    m_scintillaWidget->markerDefine(
        ScintillaMarker::ConflictIncoming,
        QVariantHash{
            {"symbol", SC_MARK_BACKGROUND},
            {"back", ScintillaWidget::colorGet(g_globalManager->warningBack2Get())}
        });
}

// private:
void ConflictWidget::contentChange() {
    m_hunk.clear();
    m_scintillaWidget->indicatorClear(ScintillaIndicator::ConflictStart);
    m_scintillaWidget->indicatorClear(ScintillaIndicator::ConflictSeparator);
    m_scintillaWidget->indicatorClear(ScintillaIndicator::ConflictEnd);
    m_scintillaWidget->markerDelete(ScintillaMarker::ConflictCurrent);
    m_scintillaWidget->markerDelete(ScintillaMarker::ConflictIncoming);
    int start = -1;
    int separator = -1;
    int end = -1;
    for (int i = 0; i < m_scintillaWidget->lineCountGet(); ++i) {
        const auto &text = m_scintillaWidget->textGet(i, 0, i, -1);
        if (text.startsWith("<<<<<<<")) start = i;
        else if (text.startsWith("=======")) separator = i;
        else if (text.startsWith(">>>>>>>")) {
            end = i;
            if (start != -1 && separator != -1 && end != -1) {
                m_hunk.insert(start, QList{start, separator, end});
                m_hunk.insert(separator, QList{start, separator, end});
                m_hunk.insert(end, QList{start, separator, end});
                m_scintillaWidget->indicatorFill(ScintillaIndicator::ConflictStart, start, 0, start, -1);
                m_scintillaWidget->indicatorFill(ScintillaIndicator::ConflictSeparator, separator, 0, separator, -1);
                m_scintillaWidget->indicatorFill(ScintillaIndicator::ConflictEnd, end, 0, end, -1);
                for (int j = start + 1; j < separator; ++j) m_scintillaWidget->markerAdd(ScintillaMarker::ConflictCurrent, j);
                for (int j = separator + 1; j < end; ++j) m_scintillaWidget->markerAdd(ScintillaMarker::ConflictIncoming, j);
            }
            start = -1;
            separator = -1;
        }
    }
}
