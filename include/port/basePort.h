#ifndef UNICOMM_BASEPORT_H
#define UNICOMM_BASEPORT_H

#include <QObject>

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr);

    ~BasePort() override = default;

    [[nodiscard]] virtual int type() = 0;

    [[nodiscard]] virtual QJsonObject config() = 0;

    [[nodiscard]] virtual QVariantHash info() = 0;

    [[nodiscard]] virtual bool open() = 0;

    virtual void close() = 0;

    virtual void clear() = 0;

    virtual void monitor(bool enabled) {
    }

    [[nodiscard]] virtual bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
        return false;
    }

    [[nodiscard]] virtual bool write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) {
        return false;
    }

    [[nodiscard]] virtual QByteArray read(int length, int timeout, const QString &rxFormat) {
        return {};
    }

    [[nodiscard]] virtual QByteArray read(int length, int timeout, const QString &peerIp, const QString &rxFormat) {
        return {};
    }

    [[nodiscard]] virtual QByteArray readUntil(const QByteArray &text, int timeout, const QString &rxFormat) {
        return {};
    }

    [[nodiscard]] virtual QByteArray readUntil(const QByteArray &text, int timeout, const QString &peerIp, const QString &rxFormat) {
        return {};
    }

signals:
    void appendLog(int type, const QString &prefix, const QString &message);

    void refreshPort(const QString &portName, const QVariantHash &session);

private:
    QThread *m_thread{};
};

#endif //UNICOMM_BASEPORT_H
