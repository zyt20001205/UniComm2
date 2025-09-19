#ifndef DATATABLE_H
#define DATATABLE_H

#include <QDockWidget>
#include <QEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMenu>
#include <QTableWidget>
#include "config.h"

class Datatable final : public QDockWidget {
    Q_OBJECT

public:
    explicit Datatable(QObject *parent = nullptr);

    ~Datatable() override = default;

    void datatableConfigSave() const;

    void datatableWrite(const QString &key, const QString &value);

    void datatableAddGraph(const QString &key);

signals:
    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y);

    void addPointDataPlot(const QString &key, double x, double y);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void datatableRename(int visualColumn);

    void datatableInsert(int visualColumn);

    void datatableRemove(int visualColumn);

    struct DataMap {
        bool enable;
        QDateTime basetime;
        QList<double> x;
        QList<double> y;
    };

    QJsonArray m_datatableConfig = g_config["datatableConfig"].toArray();
    QTableWidget *m_tableWidget = nullptr;
    QHash<QString, DataMap> m_data{};
    int m_srcIndex;
    int m_dstIndex;
};

#endif //DATATABLE_H
