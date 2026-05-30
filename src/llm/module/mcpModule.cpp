#include "llm/module/mcpModule.h"

#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardItemModel>
#include <QUrlQuery>

#include "globals.h"

// public
McpModule::McpModule(const QJsonArray &mcpConfig, QObject *parent)
    : QObject(parent),
      m_mcpModel(new QStandardItemModel(this)) {
    QNetworkRequest request{};
    request.setUrl(QUrl("https://mcp.context7.com/mcp"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json, text/event-stream");
    m_requests["Context7"] = request;
}

void McpModule::initialize() {
    const auto body = QJsonObject{
        {"jsonrpc", "2.0"},
        {"method", "initialize"},
        {
            "params", QJsonObject{
                {"protocolVersion", "2025-03-26"},
                {"capabilities", QJsonObject{}},
                {
                    "clientInfo", QJsonObject{
                        {"name", "UniComm"},
                        {"version", "0.2.0-alpha2"},
                    }
                },
            }
        },
        {"id", m_id++}
    };
    auto *reply = g_networkAccessManager->post(m_requests["Context7"], QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply] {
        if (reply->error() == QNetworkReply::NoError) {
            const auto sessionId = reply->rawHeader("Mcp-Session-Id");
            m_requests["Context7"].setRawHeader("Mcp-Session-Id", sessionId);
            const auto data = reply->readAll();
            toolsList();
        }
        reply->deleteLater();
    });
}

QString McpModule::toolsCall(const QString &owner, const QString &name, const QString &arguments) {
    const auto body = QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "tools/call"},
            {
                "params", QJsonObject{
                    {"name", name},
                    {"arguments", QJsonDocument::fromJson(arguments.toUtf8()).object()}
                }
            },
            {"id", m_id++}
    };
    auto *reply = g_networkAccessManager->post(m_requests[owner], QJsonDocument(body).toJson(QJsonDocument::Compact));

    QString result{};
    QEventLoop loop{};
    connect(reply, &QNetworkReply::finished, &loop, [&result, reply, &loop] {
        if (reply->error() == QNetworkReply::NoError) {
            const auto data = reply->readAll();
            for (const auto &line: data.split('\n')) {
                if (line.startsWith("data: ")) {
                    const auto _data = line.mid(6);
                    const auto object = QJsonDocument::fromJson(_data).object();
                    const auto content = object["result"].toObject()["content"].toArray();
                    for (const auto &part : content) {
                        const auto text = part.toObject()["text"].toString();
                        if (!text.isEmpty()) result += text;
                    }
                }
            }
        } else {
            result = reply->errorString();
        }
        reply->deleteLater();
        loop.quit();
    });
    loop.exec();
    return result;
}

void McpModule::toolsList() {
    const auto body = QJsonObject{
        {"jsonrpc", "2.0"},
        {"method", "tools/list"},
        {"params", QJsonObject{}},
        {"id", m_id++}
    };
    auto *reply = g_networkAccessManager->post(m_requests["Context7"], QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply] {
        const auto data = reply->readAll();
        for (const auto &line: data.split('\n')) {
            if (line.startsWith("data: ")) {
                const auto _data = line.mid(6);
                const auto object = QJsonDocument::fromJson(_data).object();
                const auto tools = object.value("result").toObject().value("tools").toArray();
                QJsonArray _tools{};
                for (const auto &value: tools) {
                    const auto tool = value.toObject();
                    const auto _tool = QJsonObject{
                        {"type", "function"},
                        {
                            "function", QJsonObject{
                                {"name", tool["name"]},
                                {"description", tool["description"]},
                                {"parameters", tool["inputSchema"]}
                            }
                        }
                    };
                    _tools.append(_tool);
                }
                emit registerTools("Context7", _tools);
            }
        }
        reply->deleteLater();
    });
}
