#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"
#include <QHBoxLayout>
#include <QWidget>

class Dataplot final : public QWidget {
    Q_OBJECT

public:
    explicit Dataplot(QWidget *parent = nullptr);

    ~Dataplot() override = default;

    void dataplotAppend(const QString &key);

    void dataplotAddGraph(const QList<double> &x, const QList<double> &y) const;

signals:
    void addGraphDatatable(const QString &key);

private:
    QCustomPlot *m_plot = nullptr;
};

#endif //DATAPLOT_H
