#include "api/thread.h"

#include <QThread>
#include <sol/sol.hpp>

#include "globals.h"
#include "util/luaUtils.h"

Thread::Thread(QObject *parent)
    : QObject(parent) {
}

std::string Thread::start(const sol::this_state ts, const std::string &documentPath) {
    sol::state_view lua(ts);
    const auto session = lua["session"].get<QVariantMap *>();
    const int mode = (*session)["mode"].toInt();
    QString threadId{};
    emit startThread(lua2filepath(documentPath), mode, threadId);
    return threadId.toStdString();
}

void Thread::stop(const std::string &threadId) {
    emit stopThread(QString::fromStdString(threadId));
}

void Thread::sleep(const int ms) {
    QThread::msleep(ms);
}
