#include "agent/module/mcpModule.h"

#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPromise>
#include <QSharedPointer>
#include <QStandardItem>
#include <QStringList>

#include <utility>

#include "globals.h"

// public
McpModule::McpModule(const QJsonObject &mcpConfig, QObject *parent)
    : QObject(parent),
      m_mcpModel(new McpModel(this)) {
    for (auto it = mcpConfig.begin(); it != mcpConfig.end(); ++it) {
        const auto url = QUrl(it.key());
        auto *item = new QStandardItem(url.host()); // NOLINT
        item->setData(url, McpModel::UrlRole);
        item->setData(it.value().toBool(), McpModel::EnabledRole);
        m_mcpModel->appendRow(item);
        m_servers.insert(url, item);
    }
}

void McpModule::initialize() {
    for (auto it = m_servers.cbegin(); it != m_servers.cend(); ++it) {
        if (!it.value()->data(McpModel::EnabledRole).toBool()) continue;
        initialize(it.key());
    }
}

QString McpModule::serverInsert(const QUrl &serverUrl) {
    if (!serverUrl.isValid() || serverUrl.host().isEmpty() || (serverUrl.scheme() != "http" && serverUrl.scheme() != "https")) {
        return tr("Enter a valid HTTP(S) URL.");
    }
    if (m_servers.contains(serverUrl)) return tr("This MCP server already exists.");

    auto *item = new QStandardItem(serverUrl.host()); // NOLINT
    item->setData(serverUrl, McpModel::UrlRole);
    item->setData(true, McpModel::EnabledRole);
    m_mcpModel->appendRow(item);
    m_servers.insert(serverUrl, item);
    initialize(serverUrl);
    return {};
}

void McpModule::serverRemove(const QUrl &serverUrl) {
    const auto row = m_servers.value(serverUrl)->row();
    m_servers.remove(serverUrl);
    m_mcpModel->removeRow(row);
    toolsRemove(serverUrl);
    toolsRegister();
}

void McpModule::enabledSet(const QUrl &serverUrl, const bool enabled) {
    m_servers.value(serverUrl)->setData(enabled, McpModel::EnabledRole);
    if (enabled) initialize(serverUrl);
    else {
        toolsRemove(serverUrl);
        toolsRegister();
    }
}

QFuture<QString> McpModule::toolExecute(const QString &name, const QString &arguments) {
    if (!m_tools.contains(name)) return QtFuture::makeReadyValueFuture(QString("Unknown MCP tool."));
    const auto tool = m_tools.value(name);
    const auto params = QJsonObject{
        {"name", tool.name},
        {"arguments", QJsonDocument::fromJson(arguments.toUtf8()).object()}
    };
    return request(tool.serverUrl, "tools/call", params, tool.name).then(this, [](const QJsonObject &response) {
        if (response.contains("error")) {
            return QString::fromUtf8(QJsonDocument(response.value("error").toObject()).toJson(QJsonDocument::Compact));
        }

        const auto result = response.value("result").toObject();
        QStringList text{};
        for (const auto &value: result.value("content").toArray()) {
            const auto content = value.toObject();
            if (content.value("type") == "text") text.append(content.value("text").toString());
        }
        if (text.isEmpty()) return QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));

        const auto output = text.join('\n');
        return result.value("isError").toBool() ? "MCP tool error: " + output : output;
    });
}

bool McpModule::toolContains(const QString &name) const {
    return m_tools.contains(name);
}

bool McpModule::toolReadOnly(const QString &name) const {
    return m_tools.value(name).readOnly;
}

McpModel *McpModule::mcpModelGet() const {
    return m_mcpModel;
}

// private
void McpModule::initialize(const QUrl &serverUrl) {
    request(serverUrl, "server/discover", {}).then(this, [this, serverUrl](const QJsonObject &response) {
        if (response.contains("error") || !m_servers.contains(serverUrl)) return;
        const auto result = response.value("result").toObject();
        const auto serverInfo = result.value("_meta").toObject().value("io.modelcontextprotocol/serverInfo").toObject();
        const auto name = serverInfo.value("name").toString();
        auto *item = m_servers.value(serverUrl);
        if (!name.isEmpty()) item->setText(name);
        item->setData(serverInfo.value("version").toString(), McpModel::VersionRole);
        item->setData(serverInfo.value("description").toString(), McpModel::DescriptionRole);
        item->setData(serverInfo.value("websiteUrl").toString(), McpModel::WebsiteUrlRole);
        const auto icons = serverInfo.value("icons").toArray();
        item->setData(icons.isEmpty() ? QUrl{} : QUrl(icons.first().toObject().value("src").toString()), Qt::DecorationRole);
        item->setData(result.value("instructions").toString(), McpModel::InstructionsRole);
        QStringList supportedVersions{};
        for (const auto &value: result.value("supportedVersions").toArray()) supportedVersions.append(value.toString());
        item->setData(supportedVersions, McpModel::SupportedVersionsRole);
        item->setData(result.value("capabilities").toObject().toVariantMap(), McpModel::CapabilitiesRole);
        item->setData(result.value("cacheScope").toString(), McpModel::CacheScopeRole);
        item->setData(result.value("ttlMs").toInteger(), McpModel::TtlMsRole);
        toolsRemove(serverUrl);
        toolsRegister();
        toolsGet(serverUrl, {});
    });
}

void McpModule::toolsGet(const QUrl &serverUrl, const QString &cursor) {
    QJsonObject params{};
    if (!cursor.isEmpty()) params["cursor"] = cursor;
    request(serverUrl, "tools/list", params).then(this, [this, serverUrl](const QJsonObject &response) {
        if (response.contains("error") || !m_servers.contains(serverUrl) || !m_servers.value(serverUrl)->data(McpModel::EnabledRole).toBool()) return;

        const auto result = response.value("result").toObject();
        auto prefix = m_servers.value(serverUrl)->text().toLower();
        if (prefix.isEmpty()) prefix = serverUrl.host().toLower();
        for (qsizetype index = 0; index < prefix.size(); ++index) {
            if (!prefix.at(index).isLetterOrNumber() && prefix.at(index) != '_' && prefix.at(index) != '-') prefix[index] = '_';
        }
        for (const auto &value: result.value("tools").toArray()) {
            const auto tool = value.toObject();
            const auto name = tool.value("name").toString();
            const auto exposedName = prefix + "__" + name;
            const QJsonObject definition{
                {"type", "function"},
                {
                    "function", QJsonObject{
                        {"name", exposedName},
                        {"description", tool.value("description")},
                        {"parameters", tool.value("inputSchema")}
                    }
                }
            };
            m_tools.insert(exposedName, Tool{
                serverUrl,
                name,
                definition,
                tool.value("annotations").toObject().value("readOnlyHint").toBool()
            });
        }

        const auto cursor = result.value("nextCursor").toString();
        if (cursor.isEmpty()) toolsRegister();
        else toolsGet(serverUrl, cursor);
    });
}

void McpModule::toolsRemove(const QUrl &serverUrl) {
    for (auto it = m_tools.begin(); it != m_tools.end();) {
        if (it.value().serverUrl == serverUrl) it = m_tools.erase(it);
        else ++it;
    }
}

void McpModule::toolsRegister() {
    QJsonArray tools{};
    for (const auto &tool: std::as_const(m_tools)) tools.append(tool.definition);
    emit registerTools(tools);
}

QFuture<QJsonObject> McpModule::request(const QUrl &serverUrl, const QString &method, QJsonObject params, const QString &name) {
    const auto protocolVersion = "2026-07-28";
    params["_meta"] = QJsonObject{
        {"io.modelcontextprotocol/protocolVersion", protocolVersion},
        {
            "io.modelcontextprotocol/clientInfo", QJsonObject{
                {"name", "UniComm"},
                {"version", "0.3.0-alpha1"}
            }
        },
        {"io.modelcontextprotocol/clientCapabilities", QJsonObject{}}
    };

    const auto id = m_id++;
    const auto body = QJsonObject{
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", method},
        {"params", params}
    };
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json, text/event-stream");
    request.setRawHeader("MCP-Protocol-Version", protocolVersion);
    request.setRawHeader("Mcp-Method", method.toUtf8());
    if (!name.isEmpty()) request.setRawHeader("Mcp-Name", headerValueGet(name));

    auto promise = QSharedPointer<QPromise<QJsonObject> >::create();
    promise->start();
    const auto future = promise->future();
    auto response = QSharedPointer<Response>::create();
    auto *reply = g_networkAccessManager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QIODevice::readyRead, this, [reply, response, id] {
        response->eventStream = reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("text/event-stream");
        response->buffer.append(reply->readAll());
        responseRead(*response, id, false);
    });
    connect(reply, &QNetworkReply::finished, this, [reply, promise, response, id] {
        response->eventStream = reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("text/event-stream");
        response->buffer.append(reply->readAll());
        responseRead(*response, id, true);
        if (response->object.isEmpty()) {
            response->object["error"] = QJsonObject{{"message", reply->error() == QNetworkReply::NoError ? "Invalid MCP response." : reply->errorString()}};
        }
        promise->addResult(response->object);
        promise->finish();
        reply->deleteLater();
    });
    return future;
}

QByteArray McpModule::headerValueGet(const QString &value) {
    const auto utf8 = value.toUtf8();
    auto safe = !utf8.isEmpty() && utf8.front() != ' ' && utf8.front() != '\t' && utf8.back() != ' ' && utf8.back() != '\t';
    for (const auto byte: utf8) safe = safe && byte >= 0x20 && byte <= 0x7e;
    if (utf8.startsWith("=?base64?") && utf8.endsWith("?=")) safe = false;
    return safe ? utf8 : "=?base64?" + utf8.toBase64() + "?=";
}

void McpModule::responseRead(Response &response, const int id, const bool finished) {
    if (!response.eventStream) {
        if (finished) response.object = QJsonDocument::fromJson(response.buffer).object();
        return;
    }

    if (finished && !response.buffer.endsWith("\n\n") && !response.buffer.endsWith("\r\n\r\n")) response.buffer.append("\n\n");
    while (true) {
        auto separator = response.buffer.indexOf("\r\n\r\n");
        auto separatorSize = 4;
        const auto lineFeedSeparator = response.buffer.indexOf("\n\n");
        if (separator < 0 || (lineFeedSeparator >= 0 && lineFeedSeparator < separator)) {
            separator = lineFeedSeparator;
            separatorSize = 2;
        }
        if (separator < 0) break;

        auto event = response.buffer.left(separator);
        response.buffer.remove(0, separator + separatorSize);
        event.replace("\r\n", "\n");
        QByteArray data{};
        for (const auto &line: event.split('\n')) {
            if (!line.startsWith("data:")) continue;
            auto value = line.mid(5);
            if (value.startsWith(' ')) value.remove(0, 1);
            if (!data.isEmpty()) data.append('\n');
            data.append(value);
        }
        const auto object = QJsonDocument::fromJson(data).object();
        if (object.value("id").toInt(-1) == id) response.object = object;
    }
}

QHash<int, QByteArray> McpModel::roleNames() const {
    auto roles = QStandardItemModel::roleNames();
    roles[UrlRole] = "url";
    roles[EnabledRole] = "enabled";
    roles[VersionRole] = "version";
    roles[DescriptionRole] = "description";
    roles[WebsiteUrlRole] = "websiteUrl";
    roles[InstructionsRole] = "instructions";
    roles[SupportedVersionsRole] = "supportedVersions";
    roles[CapabilitiesRole] = "capabilities";
    roles[CacheScopeRole] = "cacheScope";
    roles[TtlMsRole] = "ttlMs";
    return roles;
}
