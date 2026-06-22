#ifndef UNICOMM_TERMINALWIDGET_H
#define UNICOMM_TERMINALWIDGET_H

#include "terminal/module/vtermWidget.h"

#include <QList>
#include <QQuickPaintedItem>
#include <QTimer>

class TerminalWidget final : public QQuickPaintedItem {
    Q_OBJECT

public:
    explicit TerminalWidget(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    void fontSet(const QFont &font);

    void screenSet(int rows, int cols, const QList<VtermWidget::Cell> &cells, const VtermWidget::Cursor &cursor);

signals:
    void resize(int rows, int cols);

    void keyPressed(int key, int modifiers, const QString &text);

    void mousePressed(int row, int col, int button, int modifiers);

    void mouseReleased(int row, int col, int button, int modifiers);

    void mouseMoved(int row, int col, int button, int modifiers);

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;

    void focusOutEvent(QFocusEvent *event) override;

    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void metricsUpdate();

    void cursorBlink(bool enabled);

    QFont m_font{};
    QList<VtermWidget::Cell> m_cells{};
    VtermWidget::Cursor m_cursor{};
    QTimer m_cursorBlinkTimer{};
    bool m_cursorVisible{false};
    int m_rows{};
    int m_cols{};
    int m_requestedRows{};
    int m_requestedCols{};
    qreal m_cellWidth{1};
    qreal m_cellHeight{1};
    qreal m_ascent{1};
};

#endif //UNICOMM_TERMINALWIDGET_H
