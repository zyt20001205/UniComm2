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

    void publishDiagnostics(const QJsonArray &diagnosticsArray, const QString &scriptPath);

private:
    void jsonReturn();

    QProcess *m_process = nullptr;
    QByteArray m_buffer = {};
    int m_id = 0;
};

#endif //LUALANGUAGESERVER_H
