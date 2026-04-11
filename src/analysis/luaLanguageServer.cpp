#include "analysis/luaLanguageServer.h"

#include <QJsonArray>

#include "globals.h"
#include "util/uniCast.h"

// public
LuaLanguageServer::LuaLanguageServer(QWidget *parent)
    : QWidget(parent),
      m_process(new QProcess(this)) {
    m_process->start(QCoreApplication::applicationDirPath() + "/lua-language-server/bin/lua-language-server.exe", {});
    connect(m_process, &QProcess::readyRead, this, &LuaLanguageServer::jsonParser);
    if (!m_process->waitForStarted()) {
        qDebug() << "failed to start process";
    }
    initializeNotification();
}

LuaLanguageServer::~LuaLanguageServer() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] luals module destructed").arg(timestamp);
}

void LuaLanguageServer::propertySet(const QVariantMap &objects) {
    m_progressDialog = qvariant_cast<QObject *>(objects["lualsProgressDialog"]);
}

void LuaLanguageServer::quit() {
    if (m_process->state() != QProcess::NotRunning) {
        exitNotification();
        QEventLoop eventLoop{};
        connect(m_process, &QProcess::finished, this, [&eventLoop] { eventLoop.quit(); });
        eventLoop.exec();
    }
}

void LuaLanguageServer::jsonRequest(const QString &method, const QJsonObject &params) {
    const QJsonObject textDocument = params["textDocument"].toObject();
    const auto url = QUrl(textDocument["uri"].toString());
    if (method != "initialize" && method != "shutdown" && !textDocument["uri"].toString().endsWith(".lua")) return;
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

// private
void LuaLanguageServer::initializeNotification() {
    const QString rootUriStr = g_workspaceUrl.toString();
    const QJsonObject initializeParams{
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", rootUriStr},
        {"capabilities", QJsonObject{}}
    };
    jsonRequest("initialize", initializeParams);
    // wait until lls initialized
    QEventLoop eventLoop;
    connect(this, &LuaLanguageServer::initialized, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    jsonNotification("initialized", QJsonObject{});
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2").arg(timestamp, "workspace initialized");
}

void LuaLanguageServer::exitNotification() {
    jsonRequest("shutdown", QJsonObject{});
    // wait until lls showdowned
    QEventLoop eventLoop;
    connect(this, &LuaLanguageServer::shutdowned, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    jsonNotification("exit", QJsonObject{});
}

void LuaLanguageServer::jsonParser() {
    // append to buffer
    m_buffer.append(m_process->readAllStandardOutput());
    while (true) {
        // extract header to get pack length
        const long long headerStartIndex = m_buffer.indexOf("Content-Length: ") + 16;
        const long long headerEndIndex = m_buffer.indexOf("\r\n\r\n");
        if (headerEndIndex == -1) break;
        const auto lengthBytes = m_buffer.mid(headerStartIndex, headerEndIndex - headerStartIndex);
        const auto length = lengthBytes.toInt();
        if (m_buffer.size() < headerEndIndex + 4 + length) break;
        const auto dataBytes = m_buffer.mid(headerEndIndex + 4, lengthBytes.toInt());
        const auto json = QJsonDocument::fromJson(dataBytes).object();
        // qDebug() << json;
        m_buffer.remove(0, headerEndIndex + 4 + length);
        if (json.contains("method")) {
            // lsp notification
            const auto method = json["method"].toString();
            if (method == "textDocument/publishDiagnostics") {
                // publish diagnostics notification
                const auto params = json["params"].toObject();
                const auto diagnostics = params["diagnostics"].toArray();
                const auto uri = params["uri"].toString();
                const auto scriptUrl = uni_cast<QUrl>(uri);
                emit notificationPublishDiagnostics(scriptUrl, diagnostics);
            } else if (method == "$/hello") {
                // hello notification
                // qDebug() << json;
            } else if (method == "$/progress") {
                // progress notification
                // qDebug() << json;
                const auto params = json["params"].toObject();
                const auto token = params["token"].toInt();
                const auto value = params["value"].toObject();
                if (token == 2) {
                    const int percentage = value["percentage"].toInt(100);
                    m_progressDialog->setProperty("create2", percentage / 100.0);
                } else if (token == 3) {
                    const int percentage = value["percentage"].toInt(100);
                    m_progressDialog->setProperty("create3", percentage / 100.0);
                }
            } else if (method == "window/logMessage") {
                // log message notification
                // qDebug() << json;
                const auto params = json["params"].toObject();
                const auto message = params["message"].toString().remove("Log path: ");
                const auto url = uni_cast<QUrl>(message);
                // logging
                QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
                qDebug() << QString("[%1] Log path: %2").arg(timestamp, url.toString());
            } else if (method == "window/workDoneProgress/create") {
                // work done progress notification
                // qDebug() << json;
                const auto params = json["params"].toObject();
                const auto token = params["token"].toInt();
                if (token == 2) {
                    m_progressDialog->setProperty("done2", true);
                } else if (token == 3) {
                    m_progressDialog->setProperty("done3", true);
                }
            } else {
                qDebug() << "unknown lsp pack";
                qDebug() << json;
            }
        } else if (json.contains("id")) {
            // lsp response
            const auto id = json["id"].toInt();
            const auto method = m_methods[id];
            const auto scriptUrl = m_urls[id];
            m_methods.remove(id);
            m_urls.remove(id);
            if (method == "initialize") {
                // initialize response
                // qDebug() << json;
                emit initialized();
            } else if (method == "shutdown") {
                // shutdown response
                // qDebug() << json;
                emit shutdowned();
            } else if (method == "textDocument/codeAction") {
                // code action response
                // if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseCodeAction(scriptUrl, result);
            } else if (method == "textDocument/completion") {
                // completion response
                if (!json["result"].isObject()) return; // null result
                const auto result = json["result"].toObject();
                const auto items = result["items"].toArray();
                emit responseCompletion(scriptUrl, items);
            } else if (method == "textDocument/definition") {
                // definition response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseDefinition(scriptUrl, result);
            } else if (method == "textDocument/documentHighlight") {
                // document highlight response
                // if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseDocumentHighlight(scriptUrl, result);
            } else if (method == "textDocument/documentSymbol") {
                // document symbol response
                // if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseDocumentSymbol(scriptUrl, result);
            } else if (method == "textDocument/foldingRange") {
                // folding range response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseFoldingRange(scriptUrl, result);
            } else if (method == "textDocument/formatting") {
                // formatting response
                if (!json["result"].isArray()) {
                    emit responseFormatting(scriptUrl, QString());
                } else {
                    const auto result = json["result"].toArray();
                    const auto newText = result[0]["newText"].toString();
                    emit responseFormatting(scriptUrl, newText);
                }
            } else if (method == "textDocument/hover") {
                // hover response
                if (!json["result"].isObject()) {
                    // null result
                    emit responseHover(scriptUrl, QString());
                } else {
                    const auto result = json["result"].toObject();
                    const auto contents = result["contents"].toObject();
                    const auto value = contents["value"].toString();
                    emit responseHover(scriptUrl, value);
                }
            } else if (method == "textDocument/implementation") {
                // implementation response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseImplementation(scriptUrl, result);
            } else if (method == "textDocument/onTypeFormatting") {
                // on type formatting response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                const auto newText = result[0].toObject();
                emit responseOnTypeFormatting(scriptUrl, newText);
            } else if (method == "textDocument/rangeFormatting") {
                // range formatting response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                const auto newText = result[0]["newText"].toString();
                emit responseRangeFormatting(scriptUrl, newText);
            } else if (method == "textDocument/references") {
                // references response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseReferences(scriptUrl, result);
            } else if (method == "textDocument/semanticTokens/full") {
                // semanticTokens response
                if (!json["result"].isObject()) return; // null result
                const auto result = json["result"].toObject();
                const auto data = result["data"].toArray();
                emit responseSemanticTokens(scriptUrl, data);
            } else if (method == "textDocument/signatureHelp") {
                // signatureHelp response
                if (!json["result"].isObject()) return; // null result
                const auto result = json["result"].toObject();
                const auto signatures = result["signatures"].toArray();
                emit responseSignatureHelp(scriptUrl, signatures);
            } else if (method == "textDocument/typeDefinition") {
                // typeDefinition response
                if (!json["result"].isArray()) return; // null result
                const auto result = json["result"].toArray();
                emit responseTypeDefinition(scriptUrl, result);
            } else {
                qDebug() << "unknown lsp pack";
                qDebug() << json;
            }
        } else {
            qDebug() << "unknown lsp pack";
            qDebug() << json;
        }
        if (m_buffer.size() == 0) break;
    }
}
