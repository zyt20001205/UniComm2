#include "terminal/module/terminalWidget.h"

#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>

#include "globals.h"

TerminalWidget::TerminalWidget(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_blinkTimer(new QTimer(this)) {
    setAntialiasing(false);
    setOpaquePainting(false);
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    forceActiveFocus();
    metricsUpdate();

    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, this, [this] {
        m_blinkPhase = !m_blinkPhase;
        update();
    });
    m_blinkTimer->start();
}

void TerminalWidget::paint(QPainter *painter) {
    painter->setFont(m_font);

    const int totalRows = m_scrollback.size() + m_rows;
    const int firstRow = qMax(0, totalRows - m_rows - m_scrollOffset);

    for (int row = 0; row < m_rows; ++row) {
        const int sourceRow = firstRow + row;
        const qreal y = row * m_cellHeight + m_ascent;
        int col = 0;
        while (col < m_cols) {
            // get cell
            const auto &cell = sourceRow < m_scrollback.size()
                                   ? m_scrollback[sourceRow][col]
                                   : m_cells[(sourceRow - m_scrollback.size()) * m_cols + col];
            if (cell.width == 0) {
                ++col;
                continue;
            }
            // draw background
            const QRectF rect(col * m_cellWidth, row * m_cellHeight, cell.width * m_cellWidth, m_cellHeight);
            painter->fillRect(rect, cell.background);
            // draw cell
            if (!cell.text.isEmpty()) {
                painter->setPen(cell.foreground);
                painter->drawText(QPointF(rect.left(), y), cell.text);
            }
            // draw cursor
            if (m_scrollOffset == 0 && m_visible && m_blinkPhase && row == m_position.x() && col == m_position.y()) {
                switch (m_shape) {
                    case VTERM_PROP_CURSORSHAPE_BLOCK: {
                        painter->fillRect(rect, cell.foreground);
                        painter->setPen(cell.background);
                        painter->drawText(QPointF(rect.left(), y), cell.text);
                    }
                    break;
                    case VTERM_PROP_CURSORSHAPE_UNDERLINE: {
                        const qreal thickness = qMax<qreal>(1, m_cellHeight / 10);
                        painter->fillRect(QRectF(rect.left(), rect.bottom() - thickness, rect.width(), thickness), cell.foreground);
                    }
                    break;
                    case VTERM_PROP_CURSORSHAPE_BAR_LEFT: {
                        const qreal thickness = qMax<qreal>(1, m_cellWidth / 8);
                        painter->fillRect(QRectF(rect.left(), rect.top(), thickness, rect.height()), cell.foreground);
                    }
                    break;
                    default: break;
                }
            }
            col += cell.width;
        }
    }
}

void TerminalWidget::fontSet(const QFont &font) {
    m_font = font;
    metricsUpdate();
    update();
}

void TerminalWidget::screenSet(const int rows, const int cols, const QList<TerminalCell> &cells, const QList<QList<TerminalCell> > &scrollback) {
    m_rows = rows;
    m_cols = cols;
    m_cells = cells;
    m_scrollback = scrollback;
    m_scrollOffset = qBound(0, m_scrollOffset, m_scrollback.size());
    update();
}

void TerminalWidget::cursorPositionSet(const QPoint &position) {
    if (m_position == position) return;
    m_position = position;
    update();
}

void TerminalWidget::cursorVisibleSet(const bool visible) {
    if (m_visible == visible) return;
    m_visible = visible;
    update();
}

void TerminalWidget::cursorBlinkSet(const bool blink) {
    if (blink) {
        m_blinkTimer->start();
    } else {
        m_blinkTimer->stop();
        m_blinkPhase = true;
        update();
    }
}

void TerminalWidget::cursorShapeSet(const int shape) {
    if (m_shape == shape) return;
    m_shape = shape;
    update();
}

void TerminalWidget::cursorModeSet(const int mode) {
    if (m_mode == mode) return;
    m_mode = mode;
}

void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event->key(), event->modifiers(), event->text());
    event->accept();
}

void TerminalWidget::mousePressEvent(QMouseEvent *event) {
    forceActiveFocus();
    if (m_mode >= VTERM_PROP_MOUSE_CLICK) {
        emit mousePressed(
            event->position().y() / m_cellHeight,
            event->position().x() / m_cellWidth,
            event->button(),
            event->modifiers()
        );
    } else {
        if (event->button() == Qt::MiddleButton) {
            const QString text = QGuiApplication::clipboard()->text();
            if (!text.isEmpty()) emit keyPressed(0, Qt::NoModifier, text);
        }
    }
    event->accept();
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_mode >= VTERM_PROP_MOUSE_CLICK) {
        emit mouseReleased(
            event->position().y() / m_cellHeight,
            event->position().x() / m_cellWidth,
            event->button(),
            event->modifiers()
        );
    }
    event->accept();
}

void TerminalWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_mode == VTERM_PROP_MOUSE_MOVE || (m_mode == VTERM_PROP_MOUSE_DRAG && event->buttons() != Qt::NoButton)) {
        emit mouseMoved(
            event->position().y() / m_cellHeight,
            event->position().x() / m_cellWidth,
            event->button(),
            event->modifiers()
        );
    }
    // else qDebug() << "handle move here";
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
    update();
}

void TerminalWidget::focusOutEvent(QFocusEvent *event) {
    QQuickPaintedItem::focusOutEvent(event);
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
