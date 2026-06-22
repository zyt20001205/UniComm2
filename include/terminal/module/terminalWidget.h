#ifndef UNICOMM_TERMINALWIDGET_H
#define UNICOMM_TERMINALWIDGET_H

#include "terminal/module/vtermWidget.h"

#include <QFont>
#include <QList>
#include <QQuickPaintedItem>

class TerminalWidget final : public QQuickPaintedItem {
    Q_OBJECT

public:
    explicit TerminalWidget(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    void fontSet(const QFont &font);

    void cellsSet(const QList<VtermWidget::Cell> &cells, int rows, int cols);

    void cursorSet(const VtermWidget::Cursor &cursor);

signals:
    void resizeRequest(int rows, int cols);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void metricsUpdate();

    QFont m_font{};
    QList<VtermWidget::Cell> m_cells{};
    VtermWidget::Cursor m_cursor{};
    int m_rows{};
    int m_cols{};
    int m_requestedRows{};
    int m_requestedCols{};
    qreal m_cellWidth{1};
    qreal m_cellHeight{1};
    qreal m_ascent{1};
};

#endif //UNICOMM_TERMINALWIDGET_H
