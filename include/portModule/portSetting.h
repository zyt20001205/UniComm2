#ifndef UNICOMM_PORTSETTING_H
#define UNICOMM_PORTSETTING_H

#include <QDialog>
#include <QJsonObject>
#include <QQuickImageProvider>

class QCamera;
class QMediaCaptureSession;
class QQuickItem;
class QScreenCapture;
class QStandardItemModel;
class QVideoSink;

class ImageProvider;

class PortSetting final : public QWidget {
    Q_OBJECT

public:
    explicit PortSetting(QWidget *parent = nullptr);

    ~PortSetting() override;

    void propertySet();

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void portSettingImport(const QJsonObject &portConfig = QJsonObject());

    Q_INVOKABLE void portSettingExport();

    Q_INVOKABLE void dialogResize(int width, int height) const;

    Q_INVOKABLE void videoCapture();

    Q_INVOKABLE void previewLoad(int index) const;

    Q_INVOKABLE void roiInsert(int x, int y, int w, int h) const;

    Q_INVOKABLE void roiRemove(int index) const;

    Q_INVOKABLE void roiSwap(int src, int dst) const;

    Q_INVOKABLE void pipelineInsert(const QVariantHash &session) const;

    Q_INVOKABLE void pipelineRemove(int index) const;

    Q_INVOKABLE void pipelineSwap(int src, int dst) const;

signals:
    void insertPort(int index, const QJsonObject &portConfig);

    void editPort(const QString &oldPortName, const QJsonObject &portConfig);

private:
    void serialPortRefresh() const;

    void visaRefresh() const;

    void localHostRefresh() const;

    void videoStreamRefresh() const;

    void processRefresh(const QJsonObject &portConfig) const;

    QDialog *m_portSettingDialog{};
    QStandardItemModel *m_serialPortStandardItemModel{};
    QStandardItemModel *m_visaStandardItemModel{};
    QStandardItemModel *m_localHostStandardItemModel{};
    QStandardItemModel *m_videoStreamStandardItemModel{};
    QMediaCaptureSession *m_mediaCaptureSession{};
    QScreenCapture *m_screenCapture{};
    QCamera *m_cameraCapture{};
    QStandardItemModel *m_roiStandardItemModel{};
    QStandardItemModel *m_pipelineStandardItemModel{};
    ImageProvider *m_imageProvider{};
    QString m_oldPortName{};

    QQuickItem *m_rootItem{};
    QObject *m_swipeView{};
    // serial port
    QObject *m_serialPortNameComboBox{};
    QObject *m_serialPortBaudRateSpinBox{};
    QObject *m_serialPortDataBitsComboBox{};
    QObject *m_serialPortParityComboBox{};
    QObject *m_serialPortStopBitsComboBox{};
    // visa
    QObject *m_visaNameComboBox{};
    // tcp client
    QObject *m_tcpClientNameTextField{};
    QObject *m_tcpClientRemoteHostTextField{};
    QObject *m_tcpClientRemotePortSpinBox{};
    // ssl client
    QObject *m_sslClientNameTextField{};
    QObject *m_sslClientRemoteHostTextField{};
    QObject *m_sslClientRemotePortSpinBox{};
    // tcp server
    QObject *m_tcpServerNameTextField{};
    QObject *m_tcpServerLocalHostComboBox{};
    QObject *m_tcpServerLocalPortSpinBox{};
    // udp socket
    QObject *m_udpSocketNameTextField{};
    QObject *m_udpSocketLocalHostComboBox{};
    QObject *m_udpSocketLocalPortSpinBox{};
    QObject *m_udpSocketRemoteHostTextField{};
    QObject *m_udpSocketRemotePortSpinBox{};
    // video stream
    QObject *m_videoStreamNameComboBox{};
    // format
    QObject *m_txFormatComboBox{};
    QObject *m_txSuffixComboBox{};
    QObject *m_rxFormatComboBox{};
    // image
    QVideoSink *m_videoSink{};
    QObject *m_previewImage{};
    QObject *m_whitelistSwitch{};
    QObject *m_whitelistTextField{};
};

class ImageProvider final: public QQuickImageProvider {
    Q_OBJECT

public:
    explicit ImageProvider();

    ~ImageProvider() override = default;

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void preview(const QVideoSink* videoSink, const QJsonArray &roi, const QJsonArray &pipeline);

private:
    QPixmap m_preview{};
};

#endif //UNICOMM_PORTSETTING_H
