#ifndef UNICOMM_FILEMODULE_H
#define UNICOMM_FILEMODULE_H

#include <QVariantHash>

class QProcess;

class FileModule final : public QObject {
    Q_OBJECT

public:
    explicit FileModule(QObject *parent = nullptr);

    ~FileModule() override = default;

    void propertySet(const QVariantMap &objects);

    Q_INVOKABLE void processTerminate() const;

    Q_INVOKABLE void fileOpenInExplorer(const QUrl &fileUrl);

    Q_INVOKABLE void fileOpenInApplication(const QUrl &fileUrl);

    Q_INVOKABLE static QVariantHash fileInfo(const QUrl &fileUrl);

    Q_INVOKABLE void fileWritable(const QUrl &fileUrl, bool status);

    Q_INVOKABLE void fileNew(const QUrl &fileUrl);

    Q_INVOKABLE void fileRename(const QUrl &fileUrl, const QString &name);

    Q_INVOKABLE void fileDelete(const QUrl &fileUrl);

    Q_INVOKABLE static void copyToClipboard(const QUrl &fileUrl);

    static QString textGet(const QUrl &documentUrl, int startLine = -1, int startCharacter = -1, int endLine = -1, int endCharacter = -1);

    // void textSet();

signals:
    void appendLog(const QString &message, int type);

    void openDocument(const QUrl &documentUrl);

    void setPermission(const QUrl &documentUrl);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void didRenameFilesNotification(const QUrl &oldUrl, const QUrl &newUrl);

    void didDeleteFilesNotification(const QUrl &fileUrl);

    QObject *m_busyDialog{};
    QObject *m_messageDialog{};
    QProcess *m_process{};
};

#endif //UNICOMM_FILEMODULE_H
