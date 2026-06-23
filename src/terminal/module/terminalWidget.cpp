#include "terminal/module/terminalWidget.h"

#include <QKeyEvent>
#include <QPainter>

#include "globals.h"

TerminalWidget::TerminalWidget(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAntialiasing(false);
    setOpaquePainting(false);
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
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

    const int totalRows = m_scrollback.size() + m_rows;
    const int firstRow = qMax(0, totalRows - m_rows - m_scrollOffset);

    for (int row = 0; row < m_rows; ++row) {
        const int sourceRow = firstRow + row;
        const qreal y = row * m_cellHeight + m_ascent;
        for (int col = 0; col < m_cols; ++col) {
            // get cell
            const auto &cell = sourceRow < m_scrollback.size()
            ? m_scrollback[sourceRow][col]
            : m_cells[(sourceRow - m_scrollback.size()) * m_cols + col];
            // draw background
            if (cell.width == 0) continue;
            const QRectF rect(col * m_cellWidth, row * m_cellHeight, cell.width * m_cellWidth, m_cellHeight);
            const bool isCursor = m_scrollOffset == 0 && m_cursorVisible && row == m_cursor.x() && col == m_cursor.y();
            painter->fillRect(rect, isCursor ? cell.foreground : cell.background);
            // draw text
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

void TerminalWidget::screenSet(const int rows, const int cols, const QList<TerminalCell> &cells, const QList<QList<TerminalCell>> &scrollback, const QPoint &cursor) {
    m_rows = rows;
    m_cols = cols;
    m_cells = cells;
    m_scrollback = scrollback;
    m_scrollOffset = qBound(0, m_scrollOffset, m_scrollback.size());
    m_cursor = cursor;
    update();
}

void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event->key(), event->modifiers(), event->text());
    event->accept();
}

void TerminalWidget::mousePressEvent(QMouseEvent *event) {
    forceActiveFocus();
    emit mousePressed(
        event->position().y() / m_cellHeight,
        event->position().x() / m_cellWidth,
        event->button(),
        event->modifiers()
    );
    event->accept();
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent *event) {
    emit mouseReleased(
        event->position().y() / m_cellHeight,
        event->position().x() / m_cellWidth,
        event->button(),
        event->modifiers()
    );
    event->accept();
}

void TerminalWidget::mouseMoveEvent(QMouseEvent *event) {
    emit mouseMoved(
        event->position().y() / m_cellHeight,
        event->position().x() / m_cellWidth,
        event->button(),
        event->modifiers()
    );
    event->accept();
}

void TerminalWidget::wheelEvent(QWheelEvent *event) {
    const int degrees = event->angleDelta().y() / 120;
    const int pixels = event->pixelDelta().y() == 0 ? 0 : event->pixelDelta().y() / static_cast<int>(m_cellHeight);
    const int lines = pixels != 0 ? pixels : degrees * 3;
    if (lines == 0) {
        event->ignore();
        return;
    }

    m_scrollOffset = qBound(0, m_scrollOffset + lines, m_scrollback.size());
    update();
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
