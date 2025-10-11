#ifndef UNICOMM_EXPLORER_H
#define UNICOMM_EXPLORER_H

#include <QDockWidget>

class QFileSystemModel;
class QTreeView;

class ExplorerModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit ExplorerModule(QWidget *parent = nullptr);

    ~ExplorerModule() override = default;

    void workspaceOpen(const QUrl &rootUrl) const;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QUrl &scriptUrl);

    void runScript(const QUrl &scriptUrl, const QString &script);

    void debugScript(const QUrl &scriptUrl, const QString &script);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void scriptRun(const QModelIndex &index);

    void scriptDebug(const QModelIndex &index);

    void scriptOpen(const QModelIndex &index);

    void scriptDelete(const QModelIndex &index);

    void scriptNew();

    void scriptOpenInExplorer() const;

    QTreeView *m_explorerTreeView{};
    QFileSystemModel *m_model{};
};

#endif //UNICOMM_EXPLORER_H
