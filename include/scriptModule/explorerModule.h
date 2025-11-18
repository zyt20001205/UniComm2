#ifndef UNICOMM_EXPLORER_H
#define UNICOMM_EXPLORER_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QFileSystemModel;
class QTreeView;

class ExplorerModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ExplorerModule();

    ~ExplorerModule() override = default;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QUrl &scriptUrl);

    void runScript(const QUrl &scriptUrl, const QString &script);

    void debugScript(const QUrl &scriptUrl, const QString &script);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    void scriptRun(const QModelIndex &index);

    void scriptDebug(const QModelIndex &index);

    void scriptOpen(const QModelIndex &index);

    void scriptNew(QString rootPath = QString());

    void scriptDelete(const QModelIndex &index);

    void folderNew(QString rootPath = QString());

    void folderDelete(const QModelIndex &index);

    void scriptOpenInExplorer() const;

    QTreeView *m_explorerTreeView{};
    QFileSystemModel *m_explorerTreeModel{};
};

#endif //UNICOMM_EXPLORER_H
