#ifndef UNICOMM_LSPMANAGER_H
#define UNICOMM_LSPMANAGER_H

#include <QHash>
#include <QJsonObject>
#include <QObject>

class BaseLanguageServer;

class LSPManager final : public QObject {
    Q_OBJECT

public:
    explicit LSPManager(QObject *parent = nullptr);

    ~LSPManager() override;

    void shutdown();

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void notificationDiagnostics(const QUrl &documentUrl, const QJsonArray &diagnostics);

    void responseCodeAction(const QUrl &documentUrl, const QJsonArray &result);

    void responseCompletion(const QUrl &documentUrl, const QJsonArray &items);

    void responseDefinition(const QUrl &documentUrl, const QJsonArray &ranges);

    void responseDocumentHighlight(const QUrl &documentUrl, const QJsonArray &result);

    void responseDocumentSymbol(const QUrl &documentUrl, const QJsonArray &result);

    void responseFoldingRange(const QUrl &documentUrl, const QJsonArray &result);

    void responseFormatting(const QUrl &documentUrl, const QString &newText);

    void responseHover(const QUrl &documentUrl, const QString &message);

    void responseImplementation(const QUrl &documentUrl, const QJsonArray &ranges);

    void responseOnTypeFormatting(const QUrl &documentUrl, const QJsonObject &newText);

    void responseRangeFormatting(const QUrl &documentUrl, const QString &newText);

    void responseReferences(const QUrl &documentUrl, const QJsonArray &ranges);

    void responseSemanticTokens(const QUrl &documentUrl, const QJsonArray &data);

    void responseSignatureHelp(const QUrl &documentUrl, const QJsonArray &signature);

    void responseTypeDefinition(const QUrl &documentUrl, const QJsonArray &ranges);

private:
    void serverAdd(const QString &suffix, const QString &process);

    QHash<QString, BaseLanguageServer *> m_server{};
};

#endif //UNICOMM_LSPMANAGER_H
