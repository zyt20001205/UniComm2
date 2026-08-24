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

    Q_INVOKABLE void datatableMove(int src, int dst);

    Q_INVOKABLE void datatableClear();

    Q_INVOKABLE void datatableExport(const QString &path);

    [[nodiscard]] bool datatableWrite(const QString &key, const QString &value);

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void openFileInExplorer(const QUrl &fileUrl);

    void openFileInApplication(const QUrl &fileUrl);

    void addGraphDataPlot(const QString &key, const QList<double> &x, const QList<double> &y, int position);

    void addPointDataPlot(const QString &key, double x, double y);

private:
    struct DatatableState {
        int length{};
    };

    void _datatableInsert(int index, const QString &key);

    void _datatableRemove(const QString &key);

    void _datatableRename(const QString &oldKey, const QString &newKey);

    void _datatableMove(int src, int dst);

    void datatableCache();

    QQuickWidget *m_widget{};
    QQuickItem *m_item{};
    ToastModule *m_toast{};
    QHash<QString, int> m_datatableHash{};
    QHash<QString, DatatableState> m_datatableStates{};
    QTransposeProxyModel *m_transposeProxyModel{};
};

#endif //UNICOMM_DATATABLE_H
