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

    Q_INVOKABLE static void resourceDelete(const QUrl &resourceUrl);

    Q_INVOKABLE void resourceRename(const QUrl &resourceUrl, const QString &name) const;

    Q_INVOKABLE void resourceOpenInExplorer(const QUrl &resourceUrl);

    Q_INVOKABLE void resourceOpenInApplication(const QUrl &resourceUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &resourceUrl);

private:
    QObject* m_busyDialog{};
    QObject* m_errorDialog{};
    QProcess* m_process{};
};

#endif //UNICOMM_SYSTEMMODULE_H