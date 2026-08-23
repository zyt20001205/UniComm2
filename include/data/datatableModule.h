#ifndef UNICOMM_DATATABLE_H
#define UNICOMM_DATATABLE_H

#include <QJsonArray>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;
class QTableWidget;
class QTransposeProxyModel;

class ToastModule;

class DatatableModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatatableModule();

    ~DatatableModule() override;

    void propertySet(const QVariantHash &objects);

    static void datatableConfigSave() ;

    [[nodiscard]] QSet<QString> datatableList() const;

    Q_INVOKABLE int datatableInsert(int index, const QString &key = QString());

    Q_INVOKABLE void datatableRemove(int index);

    Q_INVOKABLE [[nodiscard]] bool datatableRename(int index, const QString &key);

    Q_INVOKABLE void datatableSwap(int src, int dst);

    Q_INVOKABLE void datatableClear();

    Q_INVOKABLE void datatableExport(const QString &path);

    [[nodiscard]] bool datatableWrite(const QString &key, const QString &value);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void addPointDataPlot(const QString &key, double x, double y);

private:
    void datatableIndex();

    QQuickWidget *m_widget{};
    QQuickItem *m_item{};
    ToastModule *m_toast{};
    QHash<QString, int> m_datatableHash{};
    QHash<QString, QVariantHash> m_datatableSession{};
    QTransposeProxyModel *m_transposeProxyModel{};
};

#endif //UNICOMM_DATATABLE_H
