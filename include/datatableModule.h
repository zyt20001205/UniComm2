#ifndef UNICOMM_DATATABLE_H
#define UNICOMM_DATATABLE_H

#include <QDockWidget>
#include <QJsonArray>

class QTableWidget;

class DatatableModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit DatatableModule(QWidget *parent = nullptr);

    ~DatatableModule() override = default;

    void datatableConfigSave() const;

    void datatableWrite(const QString &key, const QString &value);

    void datatableClear(const QString &key);

    void datatableAddGraph(const QString &key, int position);

    void datatableExport();

signals:
    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void addPointDataPlot(const QString &key, double x, double y);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void datatableRename(int visualIndex);

    void datatableInsert(int visualIndex);

    void datatableRemove(int visualIndex);

    struct DataMap {
        int index;
        bool enable;
        QDateTime basetime;
        QList<double> x;
        QList<double> y;
    };

    QJsonArray m_datatableConfig{};
    QTableWidget *m_tableWidget{};
    QHash<QString, DataMap> m_data{};
};

#endif //UNICOMM_DATATABLE_H
