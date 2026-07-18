#include "port/basePort.h"

#include <QThread>

// public
BasePort::BasePort(QObject *parent)
    : QObject(parent),
      m_thread(new QThread()) {
    this->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, this, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}
