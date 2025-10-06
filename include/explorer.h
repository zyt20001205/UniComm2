#ifndef EXPLORER_H
#define EXPLORER_H

#include <QContextMenuEvent>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QTreeView>
#include <QUrl>

extern QJsonObject g_config;

class Explorer final : public QDockWidget {
    Q_OBJECT

public:
    explicit Explorer(QWidget *parent = nullptr);

    ~Explorer() override = default;

    void workspaceOpen(const QUrl &rootUrl) const;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QString &scriptPath);

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
    QFileSystemModel *m_model = nullptr;
};

#endif //EXPLORER_H
