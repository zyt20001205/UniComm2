#ifndef UNICOMM_BASEPORT_H
#define UNICOMM_BASEPORT_H

#include <QObject>

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr);

    ~BasePort() override;

    virtual void reload(const QJsonObject &portConfig) =0;

    virtual QHash<QString, QVariant> info() = 0;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual void writeText(const QString &txText) {
    }

    virtual void writeText(const QString &txText, const QString &peerIp) {
    }

    virtual void writeData(const QByteArray &txData) {
    }

    virtual void writeData(const QByteArray &txData, const QString &peerIp) {
    }

    virtual QString readText(int timeout, int length) {
        return {};
    }

    virtual QString readText(int timeout, int length, const QString &peerIp) {
        return {};
    }

    virtual QByteArray readData(int timeout, int length) {
        return {};
    }

    virtual QByteArray readData(int timeout, int length, const QString &peerIp) {
        return {};
    }

signals:
    void appendLog(const QString &message, const QString &level);

    void showPreview(QList<QPixmap> pixmapList);

private:
    QThread *m_thread{};
};

#endif //UNICOMM_BASEPORT_H
