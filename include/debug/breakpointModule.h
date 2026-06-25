#ifndef UNICOMM_BREAKPOINTMODULE_H
#define UNICOMM_BREAKPOINTMODULE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QStandardItemModel>

class QQuickWidget;

class BreakpointModel;

class BreakpointModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit BreakpointModule();

    ~BreakpointModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    static void breakpointConfigSave();

    void breakpointInsert(const QUrl &documentUrl, int line, const QVariantHash &session);

    void breakpointRemove(const QUrl &documentUrl, int line);

    Q_INVOKABLE void breakpointReload(const QUrl &documentUrl, int line);

    Q_INVOKABLE void documentOpen(const QUrl &documentUrl);

    Q_INVOKABLE void markerAdd(const QUrl &documentUrl, int line);

    Q_INVOKABLE void breakpointDelete(const QUrl &documentUrl, int line);

    Q_INVOKABLE void breakpointsDelete(const QUrl &documentUrl);

    Q_INVOKABLE void allDelete();

    Q_INVOKABLE [[nodiscard]] static bool enabledGet(const QUrl &documentUrl, int line);

    Q_INVOKABLE static void enabledSet(const QUrl &documentUrl, int line, bool status);

    Q_INVOKABLE [[nodiscard]] static QString conditionGet(const QUrl &documentUrl, int line);

    Q_INVOKABLE static void conditionSet(const QUrl &documentUrl, int line, const QString &condition);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void openDocument(const QUrl &documentUrl);

    void addMarker(const QUrl &documentUrl, int type, int line, int time);

    void deleteMarker(const QUrl &documentUrl, int type, int line);

private:
    QQuickWidget *m_widget{};
    QObject *m_treeView{};
    BreakpointModel *m_standardItemModel{};
};

class BreakpointModel final : public QStandardItemModel {
    Q_OBJECT
    Q_PROPERTY(bool empty READ emptyGet NOTIFY emptyChanged)

public:
    explicit BreakpointModel(QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] bool emptyGet() const {
        return rowCount() == 0;
    }

signals:
    void emptyChanged();
};

#endif //UNICOMM_BREAKPOINTMODULE_H
