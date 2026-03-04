#ifndef UNICOMM_LUALANGUAGESERVER_H
#define UNICOMM_LUALANGUAGESERVER_H

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonObject>
#include <QProcess>
#include <QWidget>

class LuaLanguageServer final : public QWidget {
    Q_OBJECT

public:
    explicit LuaLanguageServer(QWidget *parent = nullptr);

    ~LuaLanguageServer() override;

    void propertySet(const QVariantMap &objects);

    void quit();

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void initialized();

    void shutdowned();

    void notificationPublishDiagnostics(const QUrl &scriptUrl, const QJsonArray &diagnostics);

    void responseCodeAction(const QUrl &scriptUrl, const QJsonArray &result);

    void responseCompletion(const QUrl &scriptUrl, const QJsonArray &items);

    void responseDefinition(const QUrl &scriptUrl, const QJsonArray &ranges);

    void responseDocumentHighlight(const QUrl &scriptUrl, const QJsonArray &result);

    void responseDocumentSymbol(const QUrl &scriptUrl, const QJsonArray &result);

    void responseFoldingRange(const QUrl &scriptUrl, const QJsonArray &result);

    void responseFormatting(const QUrl &scriptUrl, const QString &newText);

    void responseHover(const QUrl &scriptUrl, const QString &message);

    void responseImplementation(const QUrl &scriptUrl, const QJsonArray &ranges);

    void responseOnTypeFormatting(const QUrl &scriptUrl, const QJsonObject &newText);

    void responseRangeFormatting(const QUrl &scriptUrl, const QString &newText);

    void responseReferences(const QUrl &scriptUrl, const QJsonArray &ranges);

    void responseSemanticTokens(const QUrl &scriptUrl, const QJsonArray &data);

    void responseSignatureHelp(const QUrl &scriptUrl, const QJsonObject &signature);

    void responseTypeDefinition(const QUrl &scriptUrl, const QJsonArray &ranges);

private:
    void initializeNotification();

    void exitNotification();

    void jsonResponse();

    QObject* m_progressDialog{};
    QProcess *m_process{};
    QByteArray m_buffer{};
    int m_id = 0;
    QHash<int, QString> m_methods{};
    QHash<int, QUrl> m_urls{};
};

#endif //UNICOMM_LUALANGUAGESERVER_H
