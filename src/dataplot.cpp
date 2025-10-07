#include "../include/dataplot.h"

// Dataplot public
Dataplot::Dataplot(QWidget *parent)
    : QWidget(parent),
      m_plot(new QCustomPlot()),
      m_leftLegend(new QCPLegend()),
      m_rightLegend(new QCPLegend()),
      m_plotColor{
          QColor("#544559"),
          QColor("#b5aabd"),
          QColor("#aabdc4"),
          QColor("#d1c7ae"),
          QColor("#e5d9da")
      } {
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Data Plot"));
    resize(1200, 600);

    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->addWidget(m_plot);
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->yAxis2->setVisible(true);
    auto *axisRect = m_plot->axisRect();
    axisRect->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    axisRect->setRangeZoomAxes(QList<QCPAxis *>() << m_plot->xAxis,
                               QList<QCPAxis *>() << m_plot->yAxis << m_plot->yAxis2);
    axisRect->setRangeDragAxes(QList<QCPAxis *>() << m_plot->xAxis,
                               QList<QCPAxis *>() << m_plot->yAxis << m_plot->yAxis2);


    auto *insetLayout = axisRect->insetLayout();
    insetLayout->addElement(m_leftLegend, Qt::AlignTop | Qt::AlignLeft);
    insetLayout->addElement(m_rightLegend, Qt::AlignTop | Qt::AlignRight);
}

void Dataplot::dataplotAppend(const QString &key, const int position) {
    this->show();
    emit addGraphDatatable(key, position);
}

void Dataplot::dataplotAddGraph(const QString &key, const QList<double> &x, const QList<double> &y, const int position) {
    if (!m_indexHash.contains(key)) {
        // find available index
        int i = 0;
        while (true) {
            if (!m_indexSet.contains(i)) {
                m_indexSet.insert(i);
                m_indexHash[key] = i;
                if (position == 0) {
                    m_plot->addGraph();
                    m_leftLegend->addItem(new QCPPlottableLegendItem(m_leftLegend, m_plot->graph(i)));
                } else {
                    m_plot->addGraph(m_plot->xAxis, m_plot->yAxis2);
                    m_rightLegend->addItem(new QCPPlottableLegendItem(m_rightLegend, m_plot->graph(i)));
                }
                m_plot->graph(i)->setName(key);
                m_plot->graph(i)->setPen(QPen(m_plotColor[i], 2));
                break;
            }
            i++;
        }
    }
    const int index = m_indexHash[key];
    m_plot->graph(index)->setData(x, y);
    m_plot->graph(index)->rescaleAxes(true);
    m_plot->rescaleAxes();
    m_plot->replot();
    qDebug() << m_indexHash;
}

void Dataplot::dataplotAddPoint(const QString &key, const double x, const double y) {
    m_plot->graph(m_indexHash[key])->addData(x, y);
    m_plot->rescaleAxes();
    m_plot->replot();
}

// Dataplot private
