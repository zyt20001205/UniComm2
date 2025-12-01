#ifndef UNICOMM_EXPLORER_H
#define UNICOMM_EXPLORER_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QFileSystemModel;
class QQuickWidget;
class QTreeView;

class ExplorerModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ExplorerModule();

    ~ExplorerModule() override = default;

    Q_INVOKABLE void scriptRun(const QString &scriptPath);

    Q_INVOKABLE void scriptDebug(const QString &scriptPath);

    Q_INVOKABLE void scriptOpen(const QString &scriptPath);

    Q_INVOKABLE void scriptNew(QString rootPath = QString());

    Q_INVOKABLE static void scriptDelete(const QString &scriptPath);

    Q_INVOKABLE void folderNew(QString rootPath = QString());

    Q_INVOKABLE static void folderDelete(const QString &folderPath);

    Q_INVOKABLE void scriptOpenInExplorer() const;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QUrl &scriptUrl);

    void runScript(const QUrl &scriptUrl, const QString &script);

    void debugScript(const QUrl &scriptUrl, const QString &script);

private:
    QQuickWidget *m_explorerWidget{};
    QFileSystemModel *m_explorerFileModel{};
    QTreeView *m_explorerTreeView{};
};

#endif //UNICOMM_EXPLORER_H
