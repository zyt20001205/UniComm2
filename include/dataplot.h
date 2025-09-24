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

    void dataplotAppend(const QString &key, int position);

    void dataplotAddGraph(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void dataplotAddPoint(const QString &key, double x, double y);
signals:
    void addGraphDatatable(const QString &key, int position);

private:
    QCustomPlot *m_plot = nullptr;

    QCPLegend *m_leftLegend = nullptr;
    QCPLegend *m_rightLegend = nullptr;
    QHash<QString, int> m_indexHash{};
    QSet<int> m_indexSet{};
    QList<QColor> m_colors={QColor("#544559"),QColor("#b5aabd"),QColor("#aabdc4"),QColor("#d1c7ae"),QColor("#e5d9da")};
};

#endif //DATAPLOT_H
