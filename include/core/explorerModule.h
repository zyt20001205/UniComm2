#ifndef UNICOMM_EXPLORER_H
#define UNICOMM_EXPLORER_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <QSortFilterProxyModel>

class QAbstractProxyModel;
class QFileSystemModel;
class QFileSystemWatcher;
class QProcess;
class QQuickWidget;
class QStandardItemModel;

class SortFilterProxyModel;

class ExplorerModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ExplorerModule();

    ~ExplorerModule() override;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void gitUpdate() const;

    Q_INVOKABLE void toggleHidden() const;

    Q_INVOKABLE void scriptRun(const QUrl &documentUrl);

    Q_INVOKABLE void scriptDebug(const QUrl &documentUrl);

    Q_INVOKABLE void documentOpen(const QUrl &documentUrl);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void openDocument(const QUrl &documentUrl);

    void startThread(const QUrl &documentUrl, int mode, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    QQuickWidget *m_widget{};
    QFileSystemModel *m_fileSystemModel{};
    SortFilterProxyModel *m_sortFilterProxyModel{};
    QObject *m_treeView{};
    QProcess *m_process{};
    QFileSystemWatcher *m_fileWatcher{};
    QHash<QUrl, QVariant> m_documentStatus{};
};

class SortFilterProxyModel final : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit SortFilterProxyModel(const QHash<QUrl, QVariant> *documentStatus, QObject *parent = nullptr);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

private:
    const QHash<QUrl, QVariant> *m_documentStatus{};
};

#endif //UNICOMM_EXPLORER_H
