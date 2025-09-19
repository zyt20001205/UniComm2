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

void Dataplot::dataplotAddGraph(const QList<double> &x, const QList<double> &y) const {
    m_plot->graph(0)->setData(x, y);
    m_plot->graph(0)->rescaleAxes(true);
    m_plot->replot();
}

// Dataplot private
