#include "../include/dataplot.h"

// Dataplot public
Dataplot::Dataplot(QWidget *parent) : QWidget(parent), m_plot(new QCustomPlot(parent)) {
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Data Plot"));
    resize(800, 600);

    auto* layout = new QVBoxLayout(this); // NOLINT
    layout->addWidget(m_plot);

    QVector<double> x(100), y(100);
    for (int i = 0; i < 100; ++i) {
        x[i] = i / 10.0;
        y[i] = qSin(x[i]);
    }

    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->addGraph();
    m_plot->graph(0)->setData(x, y);
}
