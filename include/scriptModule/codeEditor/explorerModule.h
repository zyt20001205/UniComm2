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

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void scriptRun(const QString &scriptPath);

    Q_INVOKABLE void scriptDebug(const QString &scriptPath);

    Q_INVOKABLE void scriptOpen(const QString &scriptPath);

    Q_INVOKABLE void scriptNew(const QString &rootPath, const QString &scriptName);

    Q_INVOKABLE static void scriptDelete(const QString &scriptPath);

    Q_INVOKABLE void folderNew(const QString &rootPath, const QString &folderName);

    Q_INVOKABLE static void folderDelete(const QString &folderPath);

    Q_INVOKABLE void openInExplorer() const;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QUrl &scriptUrl);

    void startThread(const QUrl &scriptUrl, int mode, QString &threadId);

private:
    QQuickWidget *m_explorerWidget{};
    QObject* m_scriptErrorDialog{};
    QObject* m_folderErrorDialog{};
    QFileSystemModel *m_explorerFileSystemModel{};
    QTreeView *m_explorerTreeView{};
};

#endif //UNICOMM_EXPLORER_H
