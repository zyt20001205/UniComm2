#include "agent/provider/baseProvider.h"

BaseProvider::BaseProvider(QObject *parent)
    : QObject(parent),
      m_service("UniComm") {
}

QJsonObject BaseProvider::requestBuild(const QString &model, const QJsonArray &messages, const QJsonArray &tools, const bool stream) const {
    QJsonObject body{
        {"model", model},
        {"messages", messages},
        {"stream", stream},
        {"tools", tools}
    };
    if (stream) body["stream_options"] = QJsonObject{{"include_usage", true}};
    return body;
}
