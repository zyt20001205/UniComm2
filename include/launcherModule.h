#ifndef UNICOMM_LAUNCHERMODULE_H
#define UNICOMM_LAUNCHERMODULE_H

#include <QObject>

class QProcess;

class LauncherModule final : public QObject {
    Q_OBJECT

public:
    explicit LauncherModule(QObject *parent = nullptr);

    ~LauncherModule() override = default;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void processTerminate() const;

    Q_INVOKABLE void openInExplorer(const QUrl &fileUrl);

    Q_INVOKABLE void openInApplication(const QUrl &fileUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &fileUrl);

private:
    QObject* m_busyDialog{};
    QProcess* m_process{};
};

#endif //UNICOMM_LAUNCHERMODULE_H