#ifndef UNICOMM_BASEPORT_H
#define UNICOMM_BASEPORT_H

#include <QObject>

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr);

    ~BasePort() override;

    [[nodiscard]] virtual int type() = 0;

    [[nodiscard]] virtual QJsonObject config() = 0;

    [[nodiscard]] virtual QVariantHash info() = 0;

    [[nodiscard]] virtual bool open() = 0;

    virtual void close() = 0;

    virtual void clear() = 0;

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

signals:
    void appendLog(const QString &message, const QString &level);

    void refreshPort(const QString &portName, bool status);

private:
    QThread *m_thread{};
};

#endif //UNICOMM_BASEPORT_H
