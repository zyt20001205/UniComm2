#include "terminal/module/terminalWidget.h"

#include <QClipboard>
#include <QInputMethodEvent>
#include <QPainter>
#include <QTimer>

#include "globals.h"

TerminalWidget::TerminalWidget(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_blinkTimer(new QTimer(this)) {
    setAntialiasing(false);
    setOpaquePainting(false);
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    forceActiveFocus();
    metricsUpdate();

    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, this, [this] {
        m_blinkPhase = !m_blinkPhase;
        update(cursorRect());
    });
    m_blinkTimer->start();
}

void TerminalWidget::paint(QPainter *painter) {
    painter->setFont(m_font);
    QFont currentFont = m_font;

    for (int row = 0; row < m_rows; ++row) {
        const qreal y = row * m_cellHeight + m_ascent;
        int col = 0;
        while (col < m_cols) {
            // get cell
            const auto &cell = m_cells[row * m_cols + col];
            if (cell.width == 0) {
                ++col;
                continue;
            }
            // draw background
            const QRectF rect(col * m_cellWidth, row * m_cellHeight, cell.width * m_cellWidth, m_cellHeight);
            painter->fillRect(rect, cell.background);
            // draw cell
            if (!cell.text.isEmpty()) {
                QFont cellFont = m_font;
                cellFont.setBold(cell.bold);
                cellFont.setItalic(cell.italic);
                cellFont.setUnderline(cell.underline);
                cellFont.setStrikeOut(cell.strike);
                if (cellFont != currentFont) {
                    currentFont = cellFont;
                    painter->setFont(currentFont);
                }
                painter->setPen(cell.foreground);
                painter->drawText(QPointF(rect.left(), y), cell.text);
            }
            // draw cursor
            if (m_atBottom && m_visible && m_blinkPhase && row == m_position.x() && col == m_position.y()) {
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

void TerminalWidget::screenSet(const int rows, const int cols, const QList<TerminalCell> &cells, const bool atBottom) {
    m_rows = rows;
    m_cols = cols;
    m_cells = cells;
    m_atBottom = atBottom;
    update();
}

void TerminalWidget::cursorPositionSet(const QPoint &position, const QPoint &oldPosition) {
    if (m_position == position) return;
    m_position = position;
    if (!(m_atBottom && m_visible && m_blinkPhase)) return;
    update(cursorRect(oldPosition));
    update(cursorRect(position));
}

void TerminalWidget::cursorVisibleSet(const bool visible) {
    if (m_visible == visible) return;
    m_visible = visible;
    if (visible) {
        if (m_blink) {
            m_blinkPhase = false;
            m_blinkTimer->start();
        } else {
            m_blinkPhase = true;
        }
    } else {
        m_blinkTimer->stop();
        m_blinkPhase = true;
    }
    update(cursorRect());
}

void TerminalWidget::cursorBlinkSet(const bool blink) {
    if (m_blink == blink) return;
    m_blink = blink;
    if (!m_visible) return;
    if (blink) {
        m_blinkPhase = false;
        m_blinkTimer->start();
    } else {
        m_blinkTimer->stop();
        m_blinkPhase = true;
    }
    update(cursorRect());
}

void TerminalWidget::cursorShapeSet(const int shape) {
    if (m_shape == shape) return;
    m_shape = shape;
    update(cursorRect());
}

void TerminalWidget::cursorModeSet(const int mode) {
    if (m_mode == mode) return;
    m_mode = mode;
}

QVariant TerminalWidget::inputMethodQuery(const Qt::InputMethodQuery query) const {
    if (query == Qt::ImCursorRectangle) return cursorRect();
    return QQuickPaintedItem::inputMethodQuery(query);
}

// protected
void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event->key(), event->modifiers(), event->text());
    event->accept();
}

void TerminalWidget::inputMethodEvent(QInputMethodEvent *event) {
    const QString commit = event->commitString();
    if (!commit.isEmpty()) emit keyPressed(0, Qt::NoModifier, commit);
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

    if (m_mode >= VTERM_PROP_MOUSE_CLICK) {
        emit mouseWheeled(
            event->position().y() / m_cellHeight,
            event->position().x() / m_cellWidth,
            lines,
            event->modifiers()
        );
    } else {
        emit mouseScrolled(lines);
    }
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

QRect TerminalWidget::cursorRect() const {
    return cursorRect(m_position);
}

QRect TerminalWidget::cursorRect(const QPoint &position) const {
    if (m_rows < 1 || m_cols < 1) return {};
    const int row = qBound(0, position.x(), m_rows - 1);
    const int col = qBound(0, position.y(), m_cols - 1);
    return QRect(
        qFloor(col * m_cellWidth),
        qFloor(row * m_cellHeight),
        qCeil(m_cellWidth),
        qCeil(m_cellHeight)
    );
}
