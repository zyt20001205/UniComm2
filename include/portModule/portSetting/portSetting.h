#ifndef UNICOMM_PORTSETTING_H
#define UNICOMM_PORTSETTING_H

#include <QDialog>
#include <QJsonObject>

class QStandardItemModel;

class AreaSelection;

class PortSetting final : public QWidget {
    Q_OBJECT

public:
    explicit PortSetting(QWidget *parent = nullptr);

    ~PortSetting() override = default;

    void propertySet();

    Q_INVOKABLE void propertyGet(const QVariantMap &objects);

    void portSettingImport(const QJsonObject &portConfig = QJsonObject());

    Q_INVOKABLE void portSettingExport();

signals:
    void insertPort(int index, const QJsonObject &portConfig);

    void editPort(const QString &oldPortName, const QJsonObject &portConfig);

private:
    void serialPortRefresh() const;

    void visaRefresh() const;

    void localHostRefresh() const;

    void screenRefresh() const;

    void cameraRefresh() const;

    QDialog *m_portSettingDialog{};
    QStandardItemModel *m_serialPortStandardItemModel{};
    QStandardItemModel *m_visaStandardItemModel{};
    QStandardItemModel *m_localHostStandardItemModel{};
    QStandardItemModel *m_screenStandardItemModel{};
    QStandardItemModel *m_cameraStandardItemModel{};
    QString m_oldPortName{};

    QObject *m_swipeView{};
    QObject *m_tumbler{};
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
    // screen
    QObject *m_screenNameComboBox{};
    // camera
    QObject *m_cameraNameComboBox{};
    // format
    QObject *m_txFormatComboBox{};
    QObject *m_txSuffixComboBox{};
    QObject *m_rxFormatComboBox{};
};

#endif //UNICOMM_PORTSETTING_H
