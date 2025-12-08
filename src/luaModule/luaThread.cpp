#include "luaModule/luaThread.h"

#include <QThread>

#include "globals.h"

LuaThread::LuaThread(QObject *parent)
    : QObject(parent) {
}

std::string LuaThread::start(const std::string &scriptPath) {
    QString threadId{};
    emit startThread(QString::fromStdString(scriptPath), LUATHREAD_RUN, threadId);
    return threadId.toStdString();
}

void LuaThread::stop(const std::string &threadId) {
    emit stopThread(QString::fromStdString(threadId));
}

void LuaThread::sleep(const int ms) {
    QThread::msleep(ms);
}
