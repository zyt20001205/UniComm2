#include "terminal/module/terminalWidget.h"

#include <QKeyEvent>
#include <QPainter>

TerminalWidget::TerminalWidget(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAntialiasing(false);
    setOpaquePainting(false);
    setFlag(ItemHasContents, true);
    forceActiveFocus();
    m_cursorBlinkTimer.setInterval(500);
    connect(&m_cursorBlinkTimer, &QTimer::timeout, this, [this] {
        m_cursorVisible = !m_cursorVisible;
        update();
    });
    metricsUpdate();
}

void TerminalWidget::paint(QPainter *painter) {
    painter->setFont(m_font);

    for (int row = 0; row < m_rows; ++row) {
        const qreal y = row * m_cellHeight + m_ascent;
        for (int col = 0; col < m_cols; ++col) {
            const int index = row * m_cols + col;
            const auto &cell = m_cells[index];
            const QRectF rect(col * m_cellWidth, row * m_cellHeight, m_cellWidth, m_cellHeight);
            const bool isCursor = m_cursorVisible && row == m_cursor.row && col == m_cursor.col;

            painter->fillRect(rect, isCursor ? cell.foreground : cell.background);

            if (cell.text.isEmpty()) continue;
            painter->setPen(isCursor ? cell.background : cell.foreground);
            painter->drawText(QPointF(rect.left(), y), cell.text);
        }
    }
}

void TerminalWidget::fontSet(const QFont &font) {
    m_font = font;
    metricsUpdate();
    update();
}

void TerminalWidget::screenSet(const int rows, const int cols, const QList<VtermWidget::Cell> &cells, const VtermWidget::Cursor &cursor) {
    m_rows = rows;
    m_cols = cols;
    m_cells = cells;
    m_cursor = cursor;
    // if (hasActiveFocus()) {
    //     m_cursorVisible = true;
    //     cursorBlink(true);
    // }
    update();
}

void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event->key(), event->modifiers(), event->text());
    event->accept();
}

void TerminalWidget::focusInEvent(QFocusEvent *event) {
    QQuickPaintedItem::focusInEvent(event);
    m_cursorVisible = true;
    cursorBlink(true);
    update();
}

void TerminalWidget::focusOutEvent(QFocusEvent *event) {
    QQuickPaintedItem::focusOutEvent(event);
    cursorBlink(false);
    update();
}

void TerminalWidget::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    metricsUpdate();
}

void TerminalWidget::metricsUpdate() {
    QFontMetricsF metrics(m_font);
    m_cellWidth = qMax<qreal>(1, metrics.horizontalAdvance('M'));
    m_cellHeight = qMax<qreal>(1, metrics.lineSpacing());
    m_ascent = qMax<qreal>(1, metrics.ascent());

    const int rows = qMax(1, static_cast<int>(height() / m_cellHeight));
    const int cols = qMax(1, static_cast<int>(width() / m_cellWidth));
    if (rows == m_requestedRows && cols == m_requestedCols) return;

    m_requestedRows = rows;
    m_requestedCols = cols;
    emit resize(rows, cols);
}

void TerminalWidget::cursorBlink(const bool enabled) {
    if (enabled) {
        m_cursorBlinkTimer.start();
    } else {
        m_cursorBlinkTimer.stop();
        m_cursorVisible = false;
    }
}
