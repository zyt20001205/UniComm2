#ifndef UNICOMM_LUALANGUAGESERVER_H
#define UNICOMM_LUALANGUAGESERVER_H

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QWidget>

class LuaLanguageServer final : public QWidget {
    Q_OBJECT

public:
    explicit LuaLanguageServer(QWidget *parent = nullptr);

    ~LuaLanguageServer() override = default;

    void workspaceOpen(const QUrl &rootUrl);

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void initialized();

    void returnPublishDiagnostics(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void returnCompletion(const QUrl &scriptUrl, const QJsonArray &items);

    void returnDocumentSymbol(const QUrl &scriptUrl, const QJsonArray &result);

    void returnFoldingRange(const QUrl &scriptUrl, const QJsonArray &result);

    void returnFormatting(const QUrl &scriptUrl, const QString &newText);

    void returnHover(const QUrl &scriptUrl, const QString &message);

    void returnSemanticTokens(const QUrl &scriptUrl, const QJsonArray &data);

    void returnSignatureHelp(const QUrl &scriptUrl, const QJsonObject &signature);

private:
    void initializeNotification(const QUrl &rootUrl);

    void didChangeWorkspaceFoldersNotification(const QUrl &rootUrl);

    void jsonReturn();

    QProcess *m_process{};
    bool m_initialized = false;
    QUrl m_currentWorkspace{};
    QByteArray m_buffer{};
    int m_id = 0;
    QHash<int, QString> m_methods{};
    QHash<int, QUrl> m_urls{};
};

#endif //UNICOMM_LUALANGUAGESERVER_H
