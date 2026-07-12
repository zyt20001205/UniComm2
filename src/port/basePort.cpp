#include "port/basePort.h"

#include <QThread>
#include <QTime>

// public
BasePort::BasePort(QObject *parent)
    : QObject(parent),
      m_thread(new QThread()) {
    this->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}

BasePort::~BasePort() {
    // if (m_thread && m_thread->isRunning()) {
    //     m_thread->quit();
    //     m_thread->wait();
    // }
}

// protected
QString BasePort::lifetimeFormat(const qint64 elapsed) {
    const qint64 totalSeconds = elapsed / 1000;
    const qint64 days = totalSeconds / 86400;
    const int millisecondsOfDay = static_cast<int>(totalSeconds % 86400 * 1000);
    const auto time = QTime::fromMSecsSinceStartOfDay(millisecondsOfDay);
    return QStringLiteral("%1d %2")
        .arg(days)
        .arg(time.toString(QStringLiteral("hh'h' mm'm' ss's'")));
}
