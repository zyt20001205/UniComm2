#ifndef EXPLORER_H
#define EXPLORER_H

#include <QDir>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QVBoxLayout>

class Explorer final : public QDockWidget {
    Q_OBJECT

public:
    explicit Explorer(QObject *parent = nullptr);

    ~Explorer() override = default;

private:
    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_model = nullptr;

private slots:
    void fileDoubleClicked(const QModelIndex &index);

    void fileDelete(const QModelIndex &index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void appendLog(const QString &message, const QString &level);

    void loadScript(const QString &scriptPath);
};

#endif //EXPLORER_H
