#include "../include/dataplot.h"

// Dataplot public
Dataplot::Dataplot(QWidget *parent) : QWidget(parent), m_plot(new QCustomPlot(parent)) {
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Data Plot"));
    resize(800, 600);

    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->addWidget(m_plot);
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->addGraph();
}

void Dataplot::dataplotAppend(const QString &key) {
    emit addGraphDatatable(key);
}

void Dataplot::dataplotAddGraph(const QString &key, const QList<double> &x, const QList<double> &y) {
    if (!m_indexHash.contains(key)) {
        // find available index
        int i = 0;
        while (true) {
            if (!m_indexSet.contains(i)) break;
            i++;
        }
        m_indexHash[key] = i;
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
