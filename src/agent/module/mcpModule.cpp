#include "agent/module/mcpModule.h"

#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPromise>
#include <QSharedPointer>
#include <QStringList>

#include <utility>

#include "globals.h"

// public
McpModule::McpModule(const QJsonObject &mcpConfig, QObject *parent)
    : QObject(parent) {
    for (auto it = mcpConfig.begin(); it != mcpConfig.end(); ++it) {
        const auto config = it.value().toObject();
        if (config.value("enabled").toBool(true)) m_servers.insert(it.key(), QUrl(config.value("url").toString()));
    }
}

void McpModule::toolsGet() {
    m_tools.clear();
    for (auto it = m_servers.cbegin(); it != m_servers.cend(); ++it) toolsGet(it.key(), {}, {});
}

QFuture<QString> McpModule::toolExecute(const QString &name, const QString &arguments) {
    if (!m_tools.contains(name)) return QtFuture::makeReadyValueFuture(QString("Unknown MCP tool."));
    const auto tool = m_tools.value(name);
    const auto params = QJsonObject{
        {"name", tool.name},
        {"arguments", QJsonDocument::fromJson(arguments.toUtf8()).object()}
    };
    return request(tool.serverId, "tools/call", params, tool.name).then(this, [](const QJsonObject &response) {
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

// private
void McpModule::toolsGet(const QString &serverId, const QString &cursor, QJsonArray tools) {
    QJsonObject params{};
    if (!cursor.isEmpty()) params["cursor"] = cursor;
    request(serverId, "tools/list", params).then(this, [this, serverId, tools = std::move(tools)](const QJsonObject &response) mutable {
        if (response.contains("error")) return;

        const auto result = response.value("result").toObject();
        for (const auto &value: result.value("tools").toArray()) {
            const auto tool = value.toObject();
            const auto name = tool.value("name").toString();
            const auto exposedName = serverId + "__" + name;
            m_tools.insert(exposedName, Tool{serverId, name});
            tools.append(QJsonObject{
                {"type", "function"},
                {
                    "function", QJsonObject{
                        {"name", exposedName},
                        {"description", tool.value("description")},
                        {"parameters", tool.value("inputSchema")}
                    }
                }
            });
        }

        const auto cursor = result.value("nextCursor").toString();
        if (cursor.isEmpty()) emit registerTools(tools);
        else toolsGet(serverId, cursor, std::move(tools));
    });
}

QFuture<QJsonObject> McpModule::request(const QString &serverId, const QString &method, QJsonObject params, const QString &name) {
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
    QNetworkRequest request(m_servers.value(serverId));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json, text/event-stream");
    request.setRawHeader("MCP-Protocol-Version", protocolVersion);
    request.setRawHeader("Mcp-Method", method.toUtf8());
    if (!name.isEmpty()) request.setRawHeader("Mcp-Name", headerValueGet(name));

    auto promise = QSharedPointer<QPromise<QJsonObject>>::create();
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
