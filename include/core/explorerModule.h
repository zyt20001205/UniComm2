#ifndef UNICOMM_EXPLORER_H
#define UNICOMM_EXPLORER_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QFileSystemModel;
class QQuickWidget;

class ExplorerModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit ExplorerModule();

    ~ExplorerModule() override;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    Q_INVOKABLE void scriptRun(const QString &scriptPath);

    Q_INVOKABLE void scriptDebug(const QString &scriptPath);

    Q_INVOKABLE void documentOpen(const QString &scriptPath);

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void openDocument(const QUrl &documentUrl);

    void startThread(const QUrl &documentUrl, int mode, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    QQuickWidget *m_explorerWidget{};
    QFileSystemModel *m_explorerFileSystemModel{};
    QObject *m_explorerTreeView{};
};

#endif //UNICOMM_EXPLORER_H
