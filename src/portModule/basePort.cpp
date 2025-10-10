#include "portModule/basePort.h"

#include <QThread>

// BasePort public
BasePort::BasePort(QObject *parent)
    : QObject(parent),
      m_thread(new QThread()) {
    this->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
}

BasePort::~BasePort() {
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
}
