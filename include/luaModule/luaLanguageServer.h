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

    ~LuaLanguageServer() override = default;

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void initialized();

    void notificationPublishDiagnostics(const QUrl &scriptUrl, const QJsonArray &diagnosticsArray);

    void responseCompletion(const QUrl &scriptUrl, const QJsonArray &items);

    void responseDefinition(const QUrl &scriptUrl, const QJsonArray &ranges);

    void responseDocumentSymbol(const QUrl &scriptUrl, const QJsonArray &result);

    void responseFoldingRange(const QUrl &scriptUrl, const QJsonArray &result);

    void responseFormatting(const QUrl &scriptUrl, const QString &newText);

    void responseHover(const QUrl &scriptUrl, const QString &message);

    void responseSemanticTokens(const QUrl &scriptUrl, const QJsonArray &data);

    void responseSignatureHelp(const QUrl &scriptUrl, const QJsonObject &signature);

private:
    void initializeNotification();

    void jsonResponse();

    QProcess *m_process{};
    QByteArray m_buffer{};
    int m_id = 0;
    QHash<int, QString> m_methods{};
    QHash<int, QUrl> m_urls{};
};

#endif //UNICOMM_LUALANGUAGESERVER_H
