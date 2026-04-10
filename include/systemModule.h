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

    Q_INVOKABLE void fileOpenInExplorer(const QUrl &fileUrl);

    Q_INVOKABLE void fileOpenInApplication(const QUrl &fileUrl);

    Q_INVOKABLE static QVariantHash fileInfo(const QUrl &fileUrl);

    Q_INVOKABLE static void fileWritable(const QUrl &fileUrl, bool status);

    Q_INVOKABLE void fileNew(const QUrl &fileUrl);

    Q_INVOKABLE void fileRename(const QUrl &fileUrl, const QString &name);

    Q_INVOKABLE void fileDelete(const QUrl &fileUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &fileUrl);

    static QString textGet(const QUrl &scriptUrl, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    // void textSet();

signals:
    void appendLog(const QString &message, int type);

    void openScript(const QUrl &scriptUrl);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void didRenameFilesNotification(const QUrl &oldUrl, const QUrl &newUrl);

    void didDeleteFilesNotification(const QUrl &fileUrl);

    QObject *m_busyDialog{};
    QObject *m_messageDialog{};
    QProcess *m_process{};
};

#endif //UNICOMM_SYSTEMMODULE_H
