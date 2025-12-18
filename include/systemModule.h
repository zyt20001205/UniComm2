#ifndef UNICOMM_SYSTEMMODULE_H
#define UNICOMM_SYSTEMMODULE_H

#include <QObject>

class QProcess;

class SystemModule final : public QObject {
    Q_OBJECT

public:
    explicit SystemModule(QObject *parent = nullptr);

    ~SystemModule() override = default;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void processTerminate() const;

    Q_INVOKABLE static void fileDelete(const QUrl &fileUrl);

    Q_INVOKABLE void fileOpenInExplorer(const QUrl &fileUrl);

    Q_INVOKABLE void fileOpenInApplication(const QUrl &fileUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &fileUrl);

private:
    QObject* m_busyDialog{};
    QProcess* m_process{};
};

#endif //UNICOMM_SYSTEMMODULE_H