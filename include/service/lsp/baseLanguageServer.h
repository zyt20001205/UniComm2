#ifndef UNICOMM_BASELANGUAGESERVER_H
#define UNICOMM_BASELANGUAGESERVER_H

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QUrl>

class QProcess;

class BaseLanguageServer : public QObject {
    Q_OBJECT

public:
    explicit BaseLanguageServer(const QString &program, QObject *parent = nullptr);

    ~BaseLanguageServer() override;

    void initialize();

    void shutdown();

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void initialized();

    void shutdowned();

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
    void parser();

    QProcess *m_process{};
    QByteArray m_buffer{};
    int m_id{0};
    QHash<int, QString> m_methods{};
    QHash<int, QUrl> m_urls{};
};

#endif //UNICOMM_BASELANGUAGESERVER_H
