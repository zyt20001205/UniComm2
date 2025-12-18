#ifndef UNICOMM_LAUNCHERMODULE_H
#define UNICOMM_LAUNCHERMODULE_H

#include <QObject>

class LauncherModule final : public QObject {
    Q_OBJECT

public:
    explicit LauncherModule(QObject *parent = nullptr);

    ~LauncherModule() override = default;

    Q_INVOKABLE static void openInExplorer(const QUrl &fileUrl);

    Q_INVOKABLE static void openInApplication(const QUrl &fileUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &fileUrl);
};

#endif //UNICOMM_LAUNCHERMODULE_H