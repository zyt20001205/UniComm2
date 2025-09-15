#include "../include/luaLanguageServer.h"

// LuaLanguageServer public
LuaLanguageServer::LuaLanguageServer(QWidget *parent) : QWidget(parent) {
    m_process = new QProcess();
    m_process->start(QCoreApplication::applicationDirPath() + "/lua-language-server/bin/lua-language-server.exe", {});
    connect(m_process, &QProcess::readyRead, this, &LuaLanguageServer::jsonReturn);
    if (!m_process->waitForStarted()) {
        qDebug() << "failed to start process";
    }
    const QJsonObject initializeParams{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/script").toString()},
        {"capabilities", QJsonObject{}}
    };
    jsonRequest("initialize", initializeParams);
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
    m_methods.insert(m_id, method);
    qDebug() << m_methods;
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
        if (json.contains("id")) {
            // return from request
            if (const int id = json["id"].toInt(); m_methods.value(id) == "initialize") {
                // initialize request
                m_methods.remove(id);
                qDebug() << m_methods;
                // qDebug() << json;
                emit initialized();
            } else if (m_methods.value(id) == "textDocument/foldingRange") {
                // hover request
                m_methods.remove(id);
                qDebug() << m_methods;
                // qDebug() << json;
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                emit returnFoldingRange(result);
            } else if (m_methods.value(id) == "textDocument/formatting") {
                // hover request
                m_methods.remove(id);
                qDebug() << m_methods;
                // qDebug() << json;
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                const QString newText = result[0]["newText"].toString();
                emit returnFormatting(newText);
            } else if (m_methods.value(id) == "textDocument/hover") {
                // hover request
                m_methods.remove(id);
                qDebug() << m_methods;
                // qDebug() << json;
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonObject contents = result["contents"].toObject();
                const QString value = contents["value"].toString();
                emit returnHover(value);
            } else if (m_methods.value(id) == "textDocument/semanticTokens/full") {
                // semanticTokens request
                m_methods.remove(id);
                qDebug() << m_methods;
                // qDebug() << json;
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonArray data = result["data"].toArray();
                emit returnSemanticTokens(data);
            }
        } else if (json["method"].toString() == "textDocument/publishDiagnostics") {
            // return from notification
            const QJsonObject params = json["params"].toObject();
            const QJsonArray diagnosticsArray = params["diagnostics"].toArray();
            const QString uri = params["uri"].toString();
            QString scriptUri = QUrl::fromPercentEncoding(uri.toUtf8());
            if (QChar &drive = scriptUri[8]; drive.isLetter() && drive.isLower()) {
                drive = drive.toUpper();
            }
            emit returnPublishDiagnostics(scriptUri, diagnosticsArray);
        }
        if (m_buffer.size() == 0) break;
    }
}
