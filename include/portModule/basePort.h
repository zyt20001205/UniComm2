#ifndef UNICOMM_BASEPORT_H
#define UNICOMM_BASEPORT_H

#include <QObject>

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr);

    ~BasePort() override;

    virtual void reload(const QJsonObject &portConfig) =0;

    virtual std::unordered_map<std::string, std::string> info() = 0;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual bool write(const QByteArray &txData, const QString &txFormat, const QString &txSuffix) {
        return false;
    }

    virtual bool write(const QByteArray &txData, const QString &peerIp, const QString &txFormat, const QString &txSuffix) {
        return false;
    }

    virtual QByteArray read(int timeout, int length, const QString &rxFormat) {
        return {};
    }

    virtual QByteArray read(int timeout, int length, const QString &peerIp, const QString &rxFormat) {
        return {};
    }

signals:
    void appendLog(const QString &message, const QString &level);

    void togglePort(bool status);

    void showPreview(QList<QPixmap> pixmapList);

private:
    QThread *m_thread{};
};

#endif //UNICOMM_BASEPORT_H
