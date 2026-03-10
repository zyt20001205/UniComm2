#include "utils/qtUtils.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTextDocument>
#include <QUrl>

QByteArray fileHashCalc(const QString &fileInfo) {
    QFile file(fileInfo);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    if (QCryptographicHash hash(QCryptographicHash::Sha256); hash.addData(&file)) {
        return hash.result();
    }
    return {};
}

QByteArray fileHashCalc(const QUrl &fileInfo) {
    const QString filePath = fileInfo.toLocalFile();
    return fileHashCalc(filePath);
}

QString md2plain(const QString &markdown) {
    QTextDocument doc{};
    doc.setMarkdown(markdown.toHtmlEscaped());
    const QString plain = doc.toPlainText();
    return plain;
}

// public
RingBuffer::RingBuffer(const qsizetype capacity)
    : m_capacity(capacity),
      m_buffer(m_capacity, 0) {
}

qsizetype RingBuffer::write(const QByteArray &data) {
    QMutexLocker locker(&m_mutex);
    const qsizetype length = data.size();
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
    if (length < 0 || m_used == 0) return {};
    if (length == 0 || length > m_used) length = m_used;
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

void RingBuffer::clear() {
    QMutexLocker locker(&m_mutex);
    m_buffer.fill(0);
    m_readPos = 0;
    m_writePos = 0;
    m_used = 0;
}
