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
            QUrlQuery query{};
            query.addQueryItem("sessionId", sessionId);
            auto url = m_requests["Context7"].url();
            url.setQuery(query);
            m_requests["Context7"].setUrl(url);
            const auto data = reply->readAll();
            toolsList();
        }
        reply->deleteLater();
    });
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
        qDebug() << data;
        for (const auto &line: data.split('\n')) {
            if (line.startsWith("data: ")) {
                const auto _data = line.mid(6);
                const auto object = QJsonDocument::fromJson(_data).object();
                qDebug() << object;
                const auto tools = object.value("result").toObject().value("tools").toArray();
                qDebug() << tools;
            }
        }
        reply->deleteLater();
    });
}
