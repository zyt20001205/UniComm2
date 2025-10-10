#ifndef UNICOMM_DATAPLOT_H
#define UNICOMM_DATAPLOT_H

#include "qcustomplot.h"
#include <QHBoxLayout>
#include <QWidget>

class Dataplot final : public QWidget {
    Q_OBJECT

public:
    explicit Dataplot(QWidget *parent = nullptr);

    ~Dataplot() override = default;

    void dataplotAppend(const QString &key, int position);

    void dataplotAddGraph(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void dataplotAddPoint(const QString &key, double x, double y);
signals:
    void addGraphDatatable(const QString &key, int position);

private:
    QCustomPlot *m_plot{};

    QCPLegend *m_leftLegend{};
    QCPLegend *m_rightLegend{};
    QHash<QString, int> m_indexHash{};
    QSet<int> m_indexSet{};
    QList<QColor> m_plotColor={};
};

#endif //UNICOMM_DATAPLOT_H
