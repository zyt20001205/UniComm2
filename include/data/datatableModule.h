#ifndef UNICOMM_DATATABLE_H
#define UNICOMM_DATATABLE_H

#include <QJsonArray>
#include <QTransposeProxyModel>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;
class QStandardItemModel;
class QTableWidget;

class ToastModule;

class DatatableHeaderModel final : public QTransposeProxyModel {
    Q_OBJECT
    Q_PROPERTY(bool empty READ emptyGet NOTIFY emptyChanged)

public:
    explicit DatatableHeaderModel(QObject *parent = nullptr);

    [[nodiscard]] bool emptyGet() const {
        return columnCount() == 0;
    }

signals:
    void emptyChanged();
};

class DatatableModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DatatableModule();

    ~DatatableModule() override;

    void propertySet(const QVariantHash &objects);

    static void datatableConfigSave() ;

    [[nodiscard]] QSet<QString> datatableList() const;

    Q_INVOKABLE [[nodiscard]] QString datatableInsert(const QString &key = {}, const QString &targetKey = {}, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString datatableRemove(const QString &key, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString datatableRename(const QString &key, const QString &newKey, const QString &undoGroupId = {});

    Q_INVOKABLE [[nodiscard]] QString datatableMove(const QString &key, const QString &targetKey, const QString &undoGroupId = {});

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

    int datatableIndex(const QString &key) const;

    void datatableCache();

    QQuickWidget *m_widget{};
    QQuickItem *m_item{};
    ToastModule *m_toast{};
    QHash<QString, int> m_datatableHash{};
    QHash<QString, DatatableState> m_datatableStates{};
    DatatableHeaderModel *m_transposeProxyModel{};
};

#endif //UNICOMM_DATATABLE_H
