#ifndef LUALANGUAGESERVER_H
#define LUALANGUAGESERVER_H

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

    void jsonRequest(const QString &method, const QJsonObject &params);

    void jsonNotification(const QString &method, const QJsonObject &params) const;

signals:
    void initialized();

    void returnPublishDiagnostics(const QString &scriptUri, const QJsonArray &diagnosticsArray);

    void returnCompletion(const QJsonArray &items);

    void returnFoldingRange(const QJsonArray &result);

    void returnFormatting(const QString &newText);

    void returnHover(const QString &message);

    void returnSemanticTokens(const QJsonArray &data);

private:
    void jsonReturn();

    QProcess *m_process = nullptr;
    QByteArray m_buffer = {};
    int m_id = 0;
    QHash<int, QString> m_methods = {};
};

#endif //LUALANGUAGESERVER_H
