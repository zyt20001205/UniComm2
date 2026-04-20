#include "api/thread.h"

#include <QThread>
#include <sol/sol.hpp>

#include "globals.h"
#include "util/uniCast.h"

Thread::Thread(QObject *parent)
    : QObject(parent) {
}

std::string Thread::start(const sol::this_state ts, const std::string &documentPath) {
    sol::state_view lua(ts);
    const auto session = lua["session"].get<QVariantMap *>();
    const int mode = (*session)["mode"].toInt();
    QString threadId{};
    const LPath luaPath = QString::fromStdString(documentPath);
    emit startThread(uni_cast<QUrl>(luaPath), mode, threadId, -1, -1, -1, -1);
    return threadId.toStdString();
}

void Thread::stop(const std::string &threadId) {
    emit stopThread(QString::fromStdString(threadId));
}

void Thread::sleep(const int ms) {
    QThread::msleep(ms);
}
