#ifndef UNICOMM_DATATABLE_H
#define UNICOMM_DATATABLE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;
class QTableWidget;

class DatatableModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatatableModule();

    ~DatatableModule() override;

    void propertySet(const QVariantMap &objects);

    void datatableConfigSave() const;

    void datatableList(QSet<QString> &datatableList) const;

    Q_INVOKABLE void datatableInsert(int index, const QString &key);

    Q_INVOKABLE void datatableRemove(int index);

    Q_INVOKABLE void datatableRename(int index, const QString &key);

    Q_INVOKABLE void datatableSwap(int src, int dst);

    Q_INVOKABLE void datatableClear();

    Q_INVOKABLE void datatableExport(const QString &fileName);

    void datatableWrite(QEventLoop *eventloop, bool *status, const QString &key, const QString &value);

signals:
    void appendLog(const QString &message, const QString &level);

    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void addPointDataPlot(const QString &key, double x, double y);

private:
    void datatableIndex();

    QQuickWidget *m_datatableWidget{};
    QQuickItem *m_rootItem{};
    QHash<QString, int> m_datatableHash{};
    QHash<QString, QVariantHash> m_datatableSession{};
};

#endif //UNICOMM_DATATABLE_H
