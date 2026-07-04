#include "port/module/ringBuffer.h"

#include <QDebug>

// public
RingBuffer::RingBuffer(const qsizetype capacity)
    : m_capacity(capacity),
      m_buffer(m_capacity, 0) {
}

qsizetype RingBuffer::write(const QByteArray &data) {
    const qsizetype length = data.size();
    QMutexLocker locker(&m_mutex);
    if (length == 0 || length > m_capacity - m_used) return 0;
    const qsizetype firstChunk = qMin(length, m_capacity - m_writePos);
    const qsizetype secondChunk = length - firstChunk;
    if (firstChunk > 0) m_buffer.replace(m_writePos, firstChunk, data.constData(), firstChunk);
    if (secondChunk > 0) m_buffer.replace(static_cast<qsizetype>(0), secondChunk, data.constData() + firstChunk, secondChunk);

    m_writePos = (m_writePos + length) % m_capacity;
    m_used += length;
    return length;
}

QByteArray RingBuffer::read(qsizetype length) {
    QMutexLocker locker(&m_mutex);
    if (length < 0 || length > m_used) return {};
    if (length == 0) length = m_used;
    QByteArray data;
    data.reserve(length);
    const qsizetype firstChunk = qMin(length, m_capacity - m_readPos);
    const qsizetype secondChunk = length - firstChunk;
    if (firstChunk > 0) data.append(m_buffer.constData() + m_readPos, firstChunk);
    if (secondChunk > 0) data.append(m_buffer.constData(), secondChunk);

    m_readPos = (m_readPos + length) % m_capacity;
    m_used -= length;
    return data;
}

qsizetype RingBuffer::used() {
    QMutexLocker locker(&m_mutex);
    return m_used;
}

qsizetype RingBuffer::distance(const QByteArray &text) {
    QMutexLocker locker(&m_mutex);
    const auto size = text.size();
    if (m_used == 0 || text.isEmpty() || m_used < size) return -1;
    qsizetype index = 0;
    index = m_buffer.indexOf(text, m_readPos);
    if (index == -1) index = m_buffer.indexOf(text);
    qsizetype length = 0;
    if (index >= m_readPos) {
        length = index + size - m_readPos;
    } else {
        length = m_capacity - m_readPos + index + size;
    }
    if (length > m_used) return -1;
    return length;
}

void RingBuffer::clear() {
    QMutexLocker locker(&m_mutex);
    m_buffer.fill(0);
    m_readPos = 0;
    m_writePos = 0;
    m_used = 0;
}
