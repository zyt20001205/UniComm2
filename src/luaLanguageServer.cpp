#include "../include/luaLanguageServer.h"

// LuaLanguageServer public
LuaLanguageServer::LuaLanguageServer(QWidget *parent) : QWidget(parent) {
    m_process = new QProcess();
    m_process->start(QCoreApplication::applicationDirPath() + "/lua-language-server/bin/lua-language-server.exe", {});
    connect(m_process, &QProcess::readyRead, this, &LuaLanguageServer::jsonReturn);
    if (!m_process->waitForStarted()) {
        qDebug() << "failed to start process";
    }
    const QJsonObject initParams{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/script").toString()},
        {"capabilities", QJsonObject{}}
    };
    jsonRequest("initialize", initParams);
    // wait until lls initialized
    QEventLoop loop;
    connect(this, &LuaLanguageServer::initialized, &loop, &QEventLoop::quit);
    loop.exec();
    jsonNotification("initialized", QJsonObject{});
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "lls module initialized");
}

void LuaLanguageServer::jsonRequest(const QString &method, const QJsonObject &params) {
    const QJsonObject msg = {
        {"jsonrpc", "2.0"},
        {"id", m_id},
        {"method", method},
        {"params", params}
    };
    m_id++;
    const QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    const QByteArray header = "Content-Length: " + QByteArray::number(data.size()) + "\r\n\r\n";
    m_process->write(header);
    m_process->write(data);
    // qDebug() << msg;
}

void LuaLanguageServer::jsonNotification(const QString &method, const QJsonObject &params) const {
    const QJsonObject msg = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    const QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    const QByteArray header = "Content-Length: " + QByteArray::number(data.size()) + "\r\n\r\n";
    m_process->write(header);
    m_process->write(data);
    // qDebug() << msg;
}

// LuaLanguageServer private
void LuaLanguageServer::jsonReturn() {
    while (true) {
        // append to buffer
        m_buffer.append(m_process->readAllStandardOutput());
        // extract header to get pack length
        const int headerFrontIndex = m_buffer.indexOf("Content-Length: ") + 16;
        const int headerEndIndex = m_buffer.indexOf("\r\n\r\n");
        const QByteArray lengthBytes = m_buffer.mid(headerFrontIndex, headerEndIndex - headerFrontIndex);
        const QByteArray dataBytes = m_buffer.mid(headerEndIndex + 4, lengthBytes.toInt());
        const QJsonObject json = QJsonDocument::fromJson(dataBytes).object();
        // qDebug() << json;
        m_buffer.remove(0, headerEndIndex + 4 + lengthBytes.toInt());
        // id 0: initialize request
        if (json.contains("id") && json["id"].toInt() == 0 && json.contains("result")) {
            emit initialized();
        } else if (json["method"].toString() == "textDocument/publishDiagnostics") {
            const QJsonObject params = json["params"].toObject();
            const QJsonArray diagnosticsArray = params["diagnostics"].toArray();
            const QUrl scriptUri(params["uri"].toString());
            const QString scriptAbsolutePath = scriptUri.toLocalFile();
            const QFileInfo fileInfo(scriptAbsolutePath);
            QString scriptPath = fileInfo.fileName();
            emit publishDiagnostics(diagnosticsArray, scriptPath);
        }
        if (m_buffer.size() == 0) break;
    }
}
