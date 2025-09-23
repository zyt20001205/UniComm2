#include "../include/dataplot.h"

// Dataplot public
Dataplot::Dataplot(QWidget *parent) : QWidget(parent), m_plot(new QCustomPlot(parent)) {
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Data Plot"));
    resize(800, 600);

    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->addWidget(m_plot);
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->yAxis2->setVisible(true);
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
                if (position != 1) {
                    m_plot->addGraph();
                }
                else {
                    m_plot->addGraph(m_plot->xAxis, m_plot->yAxis2);
                }
                break;
            }
            i++;
        }
    }
    m_plot->graph(m_indexHash[key])->setData(x, y);
    m_plot->graph(m_indexHash[key])->rescaleAxes(true);
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
