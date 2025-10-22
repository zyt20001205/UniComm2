#include "luaModule/luaLanguageServer.h"

#include <QJsonArray>

#include "globals.h"

// LuaLanguageServer public
LuaLanguageServer::LuaLanguageServer(QWidget *parent)
    : QWidget(parent),
      m_process(new QProcess(this)) {
    m_process->start(QCoreApplication::applicationDirPath() + "/lua-language-server/bin/lua-language-server.exe", {});
    connect(m_process, &QProcess::readyRead, this, &LuaLanguageServer::jsonReturn);
    if (!m_process->waitForStarted()) {
        qDebug() << "failed to start process";
    }
}

void LuaLanguageServer::workspaceOpen(const QUrl &rootUrl) {
    if (m_rootUrl.isEmpty()) {
        initializeNotification(rootUrl);
    } else {
        didChangeWorkspaceFoldersNotification(rootUrl);
    }
    m_rootUrl = rootUrl;
}

void LuaLanguageServer::jsonRequest(const QString &method, const QJsonObject &params) {
    const QJsonObject textDocument = params["textDocument"].toObject();
    const auto url = QUrl(textDocument["uri"].toString());
    m_methods.insert(m_id, method);
    m_urls.insert(m_id, url);
    const QJsonObject msg = {
        {"jsonrpc", "2.0"},
        {"id", m_id},
        {"method", method},
        {"params", params}
    };
    // qDebug() << m_methods;
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
void LuaLanguageServer::initializeNotification(const QUrl &rootUrl) {
    const QString rootUriStr = rootUrl.toString();
    const QJsonObject initializeParams{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", rootUriStr},
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
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace initialized");
}

void LuaLanguageServer::didChangeWorkspaceFoldersNotification(const QUrl &rootUrl) const {
    if (rootUrl == m_rootUrl) return;
    const QString rootUriStr = rootUrl.toString();
    const QJsonObject didChangeWorkspaceFoldersParams{
        {
            "event", QJsonObject{
                {
                    "added", QJsonArray{
                        QJsonObject{
                            {"uri", rootUriStr}
                        }
                    }
                },
                {
                    "removed", QJsonArray{
                        QJsonObject{
                            {"uri", m_rootUrl.toString()}
                        }
                    }
                }
            }
        }
    };
    jsonNotification("workspace/didChangeWorkspaceFolders", didChangeWorkspaceFoldersParams);
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace loaded");
}

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
            const int id = json["id"].toInt();
            const QString method = m_methods[id];
            const QUrl scriptUrl = m_urls[id];
            m_methods.remove(id);
            m_urls.remove(id);
            if (method == "initialize") {
                // initialize request
                emit initialized();
            } else if (method == "textDocument/completion") {
                // completion request
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonArray items = result["items"].toArray();
                emit returnCompletion(scriptUrl, items);
            } else if (method == "textDocument/definition") {
                // definition request
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                emit returnDefinition(scriptUrl, result);
            } else if (method == "textDocument/documentSymbol") {
                // document symbol request
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                emit returnDocumentSymbol(scriptUrl, result);
            } else if (method == "textDocument/foldingRange") {
                // folding range request
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                emit returnFoldingRange(scriptUrl, result);
            } else if (method == "textDocument/formatting") {
                // formatting request
                if (!json["result"].isArray()) return; // null result
                const QJsonArray result = json["result"].toArray();
                const QString newText = result[0]["newText"].toString();
                emit returnFormatting(scriptUrl, newText);
            } else if (method == "textDocument/hover") {
                // hover request
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonObject contents = result["contents"].toObject();
                const QString value = contents["value"].toString();
                emit returnHover(scriptUrl, value);
            } else if (method == "textDocument/semanticTokens/full") {
                // semanticTokens request
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonArray data = result["data"].toArray();
                emit returnSemanticTokens(scriptUrl, data);
            } else if (method == "textDocument/signatureHelp") {
                // signatureHelp request
                if (!json["result"].isObject()) return; // null result
                const QJsonObject result = json["result"].toObject();
                const QJsonArray signatures = result["signatures"].toArray();
                const QJsonObject signature = signatures[0].toObject();
                emit returnSignatureHelp(scriptUrl, signature);
            }
        } else if (json["method"].toString() == "textDocument/publishDiagnostics") {
            // publish diagnostics return
            const QJsonObject params = json["params"].toObject();
            const QJsonArray diagnosticsArray = params["diagnostics"].toArray();
            QString uri = params["uri"].toString();
            uri = QUrl::fromPercentEncoding(uri.toUtf8());
            if (QChar &drive = uri[8]; drive.isLetter() && drive.isLower()) {
                drive = drive.toUpper();
            }
            const QUrl scriptUrl(uri);
            emit returnPublishDiagnostics(scriptUrl, diagnosticsArray);
        } else {
            qDebug() << "unknown lsp pack";
            qDebug() << json;
        }
        if (m_buffer.size() == 0) break;
    }
}
