#ifndef UNICOMM_RINGBUFFER_H
#define UNICOMM_RINGBUFFER_H

#include <QMutex>

class RingBuffer final {
public:
    explicit RingBuffer(qsizetype capacity);

    [[nodiscard]] qsizetype write(const QByteArray &data);

    [[nodiscard]] QByteArray read(qsizetype length);

    [[nodiscard]] qsizetype used();

    [[nodiscard]] qsizetype distance(const QByteArray &text);

    void clear();

private:
    const qsizetype m_capacity{};
    QByteArray m_buffer{};
    qsizetype m_readPos{};
    qsizetype m_writePos{};
    qsizetype m_used{};
    QMutex m_mutex{};
};

#endif //UNICOMM_RINGBUFFER_H