#include "service/lsp/lspManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>

#include "service/lsp/languageServer.h"

// public
LSPManager::LSPManager(QObject *parent)
    : QObject(parent) {
    const QDir rootDir(QCoreApplication::applicationDirPath());
    serverAdd("lua",
        rootDir.absoluteFilePath("lua-language-server/bin/lua-language-server.exe"),
        {"--configpath", rootDir.absoluteFilePath("lua-language-server/meta/3rd/UniComm/.luarc.json")});
}

LSPManager::~LSPManager() {
    const auto timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] lsp manager destructed").arg(timestamp);
}

void LSPManager::shutdown() {
    for (const auto &session: m_session) {
        session.server->shutdown();
    }
}

void LSPManager::jsonRequest(const QString &method, const QJsonObject &params) {
    const QJsonObject textDocument = params["textDocument"].toObject();
    const QUrl url(textDocument["uri"].toString());
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();

    const auto it = m_session.find(suffix);
    if (it != m_session.end()) it.value().server->jsonRequest(method, params);
}

void LSPManager::jsonNotification(const QString &method, const QJsonObject &params) const {
    // workspace notification
    if (method.startsWith("workspace/")) {
        for (const auto &session: m_session) {
            session.server->jsonNotification(method, params);
        }
        return;
    }
    // file notification
    const QJsonObject textDocument = params["textDocument"].toObject();
    const QUrl url(textDocument["uri"].toString());
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();
    const auto it = m_session.find(suffix);
    if (it != m_session.end()) it.value().server->jsonNotification(method, params);
}

// private
void LSPManager::serverAdd(const QString &suffix, const QString &process, const QStringList &arguments) {
    auto *server = new LanguageServer(process, arguments, this);
    connect(server, &LanguageServer::notificationDiagnostics, this, &LSPManager::notificationDiagnostics);
    connect(server, &LanguageServer::responseCodeAction, this, &LSPManager::responseCodeAction);
    connect(server, &LanguageServer::responseCompletion, this, &LSPManager::responseCompletion);
    connect(server, &LanguageServer::responseDefinition, this, &LSPManager::responseDefinition);
    connect(server, &LanguageServer::responseDocumentHighlight, this, &LSPManager::responseDocumentHighlight);
    connect(server, &LanguageServer::responseDocumentSymbol, this, &LSPManager::responseDocumentSymbol);
    connect(server, &LanguageServer::responseFoldingRange, this, &LSPManager::responseFoldingRange);
    connect(server, &LanguageServer::responseFormatting, this, &LSPManager::responseFormatting);
    connect(server, &LanguageServer::responseHover, this, &LSPManager::responseHover);
    connect(server, &LanguageServer::responseImplementation, this, &LSPManager::responseImplementation);
    connect(server, &LanguageServer::responseOnTypeFormatting, this, &LSPManager::responseOnTypeFormatting);
    connect(server, &LanguageServer::responsePrepareRename, this, &LSPManager::responsePrepareRename);
    connect(server, &LanguageServer::responseRangeFormatting, this, &LSPManager::responseRangeFormatting);
    connect(server, &LanguageServer::responseReferences, this, &LSPManager::responseReferences);
    connect(server, &LanguageServer::responseRename, this, &LSPManager::responseRename);
    connect(server, &LanguageServer::responseSemanticTokens, this, &LSPManager::responseSemanticTokens);
    connect(server, &LanguageServer::responseSignatureHelp, this, &LSPManager::responseSignatureHelp);
    connect(server, &LanguageServer::responseTypeDefinition, this, &LSPManager::responseTypeDefinition);
    connect(server, &LanguageServer::initialized, this, [this, suffix, server](const QJsonObject &params) {
        LanguageServerSession session{};
        session.server = server;
        const auto result = params["result"].toObject();
        const auto capabilities = result["capabilities"].toObject();
        // completion trigger
        for (const auto &value: capabilities["completionProvider"].toObject()["triggerCharacters"].toArray()) {
            const auto trigger = value.toString();
            if (!trigger.isEmpty()) session.completionTrigger.insert(trigger.front());
        }
        // format trigger
        {
            const auto trigger = capabilities["documentOnTypeFormattingProvider"].toObject()["firstTriggerCharacter"].toString();
            if (!trigger.isEmpty()) session.formattingTrigger.insert(trigger.front());
        }
        // signature help trigger
        for (const auto &value: capabilities["signatureHelpProvider"].toObject()["triggerCharacters"].toArray()) {
            const auto trigger = value.toString();
            if (!trigger.isEmpty()) session.signatureHelpTrigger.insert(trigger.front());
        }
        m_session.insert(suffix.toLower(), session);
    });
    server->initialize();
}
