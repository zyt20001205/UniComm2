#ifndef UNICOMM_FILEMODULE_H
#define UNICOMM_FILEMODULE_H

#include <QVariantHash>

class QProcess;

class FileModule final : public QObject {
    Q_OBJECT

public:
    explicit FileModule(QObject *parent = nullptr);

    ~FileModule() override = default;

    void propertySet(const QVariantHash &objects);

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
    void appendLog(int type, const QString &prefix, const QString &message);

    void openDocument(const QUrl &documentUrl);

    void setPermission(const QUrl &documentUrl);

    void appendBackground(int &taskId, const std::function<void()> &abort);

    void removeBackground(int taskId);

    void refreshBackground(int taskId, const QString &message);

    void notificationJson(const QString &method, const QJsonObject &params);

private:
    void processTerminate() const;

    void didRenameFilesNotification(const QUrl &oldUrl, const QUrl &newUrl);

    void didDeleteFilesNotification(const QUrl &fileUrl);

    QObject *m_messageDialog{};
    int m_taskId{-1};
    QProcess *m_process{};
};

#endif //UNICOMM_FILEMODULE_H
