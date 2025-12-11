#include "luaModule/luaThread.h"

#include <QThread>
#include <sol/sol.hpp>

#include "globals.h"

LuaThread::LuaThread(QObject *parent)
    : QObject(parent) {
}

std::string LuaThread::start(const sol::this_state ts, const std::string &scriptPath) {
    sol::state_view lua(ts);
    const auto session = lua["session"].get<QVariantMap *>();
    const int mode = (*session)["mode"].toInt();
    QString threadId{};
    emit startThread(QString::fromStdString(scriptPath), mode, threadId);
    return threadId.toStdString();
}

void LuaThread::stop(const std::string &threadId) {
    emit stopThread(QString::fromStdString(threadId));
}

void LuaThread::sleep(const int ms) {
    QThread::msleep(ms);
}
