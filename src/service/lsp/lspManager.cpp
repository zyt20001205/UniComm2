#include "service/lsp/lspManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QUrl>

#include "service/lsp/baseLanguageServer.h"

// public
LSPManager::LSPManager(QObject *parent)
    : QObject(parent) {
    const QDir rootDir(QCoreApplication::applicationDirPath());
    serverAdd("lua", rootDir.absoluteFilePath("lua-language-server/bin/lua-language-server.exe"));
}

LSPManager::~LSPManager() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] lsp manager destructed").arg(timestamp);
}

void LSPManager::shutdown() {
    for (auto *server: m_server) {
        server->shutdown();
    }
}

void LSPManager::jsonRequest(const QString &method, const QJsonObject &params) {
    const QJsonObject textDocument = params["textDocument"].toObject();
    const QUrl url(textDocument["uri"].toString());
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();

    const auto it = m_server.find(suffix);
    if (it != m_server.end()) it.value()->jsonRequest(method, params);
}

void LSPManager::jsonNotification(const QString &method, const QJsonObject &params) const {
    // workspace notification
    if (method.startsWith("workspace/")) {
        for (const auto *server: m_server) {
            server->jsonNotification(method, params);
        }
        return;
    }
    // file notification
    const QJsonObject textDocument = params["textDocument"].toObject();
    const QUrl url(textDocument["uri"].toString());
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();
    const auto it = m_server.find(suffix);
    if (it != m_server.end()) it.value()->jsonNotification(method, params);
}

// private
void LSPManager::serverAdd(const QString &suffix, const QString &process) {
    auto *server = new BaseLanguageServer(process, this);
    m_server.insert(suffix.toLower(), server);
    connect(server, &BaseLanguageServer::notificationDiagnostics, this, &LSPManager::notificationDiagnostics);
    connect(server, &BaseLanguageServer::responseCodeAction, this, &LSPManager::responseCodeAction);
    connect(server, &BaseLanguageServer::responseCompletion, this, &LSPManager::responseCompletion);
    connect(server, &BaseLanguageServer::responseDefinition, this, &LSPManager::responseDefinition);
    connect(server, &BaseLanguageServer::responseDocumentHighlight, this, &LSPManager::responseDocumentHighlight);
    connect(server, &BaseLanguageServer::responseDocumentSymbol, this, &LSPManager::responseDocumentSymbol);
    connect(server, &BaseLanguageServer::responseFoldingRange, this, &LSPManager::responseFoldingRange);
    connect(server, &BaseLanguageServer::responseFormatting, this, &LSPManager::responseFormatting);
    connect(server, &BaseLanguageServer::responseHover, this, &LSPManager::responseHover);
    connect(server, &BaseLanguageServer::responseImplementation, this, &LSPManager::responseImplementation);
    connect(server, &BaseLanguageServer::responseOnTypeFormatting, this, &LSPManager::responseOnTypeFormatting);
    connect(server, &BaseLanguageServer::responseRangeFormatting, this, &LSPManager::responseRangeFormatting);
    connect(server, &BaseLanguageServer::responseReferences, this, &LSPManager::responseReferences);
    connect(server, &BaseLanguageServer::responseSemanticTokens, this, &LSPManager::responseSemanticTokens);
    connect(server, &BaseLanguageServer::responseSignatureHelp, this, &LSPManager::responseSignatureHelp);
    connect(server, &BaseLanguageServer::responseTypeDefinition, this, &LSPManager::responseTypeDefinition);
}
