#ifndef UNICOMM_DATATABLE_H
#define UNICOMM_DATATABLE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QTableWidget;

class DatatableModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatatableModule();

    ~DatatableModule() override;

    void datatableConfigSave() const;

    QVariantList datatableList() const;

    void datatableInsert(int visualIndex, QString key = QString());

    void datatableAnnotate() const;

    bool datatableWrite(const QString &key, const QString &value);

    bool datatableClear(const QString &key);

    void datatableAddGraph(const QString &key, int position);

    void datatableExport();

    QHash<QString, int> m_datatableHash{};

signals:
    void appendLog(const QString &message, const QString &level);

    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void addPointDataPlot(const QString &key, double x, double y);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void datatableRename(int visualIndex);

    void datatableRemove(int visualIndex);

    void datatableSwap(int logicalIndex, int oldVisualIndex, int newVisualIndex);

    struct DataMap {
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
