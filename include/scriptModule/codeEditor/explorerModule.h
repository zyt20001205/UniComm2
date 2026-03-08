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

    Q_INVOKABLE void scriptOpen(const QString &scriptPath);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void appendLog(const QString &message, const QString &level);

    void openScript(const QUrl &scriptUrl);

    void startThread(const QUrl &scriptUrl, int mode, QString &threadId, int startLine, int startCharacter, int endLine, int endCharacter);

private:
    QQuickWidget *m_explorerWidget{};
    QFileSystemModel *m_explorerFileSystemModel{};
    QObject *m_explorerTreeView{};
};

#endif //UNICOMM_EXPLORER_H
