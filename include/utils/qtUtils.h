#ifndef UNICOMM_QTUTILS_H
#define UNICOMM_QTUTILS_H

#include<QMutexLocker>

class QUrl;

QByteArray fileHashCalc(const QString &fileInfo);

QByteArray fileHashCalc(const QUrl &fileInfo);

QString md2plain(const QString &markdown);

class RingBuffer final {
public:
    explicit RingBuffer(qsizetype capacity);

    [[nodiscard]] qsizetype write(const QByteArray &data);

    [[nodiscard]] QByteArray read(qsizetype length);

    [[nodiscard]] qsizetype used();

    void clear();

private:
    const qsizetype m_capacity{};
    QByteArray m_buffer{};
    qsizetype m_readPos{};
    qsizetype m_writePos{};
    qsizetype m_used{};
    QMutex m_mutex{};
};

#endif //UNICOMM_QTUTILS_H
