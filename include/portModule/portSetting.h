#ifndef UNICOMM_PORTSETTING_H
#define UNICOMM_PORTSETTING_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;

class AreaSelection;

class PortSetting final : public QDialog {
    Q_OBJECT

public:
    explicit PortSetting(QWidget *parent = nullptr);

    ~PortSetting() override = default;

    void portSettingLoad(const QJsonObject &portConfig);


signals:
    void savePortSetting(const QJsonObject &portConfig);

private:
    void portSettingHideAll() const;

    void portSettingTypeSwitch(int portType);

    void portSettingSave(int portType);

    QVBoxLayout *m_portSettingLayout{};
    QWidget *m_portTypeWidget{};
    QComboBox *m_portTypeCombobox{};
    // serial port
    QWidget *m_serialPortNameWidget{};
    QComboBox *m_serialPortNameCombobox{};
    QWidget *m_serialPortBaudRateWidget{};
    QSpinBox *m_serialPortBaudRateSpinBox{};
    QWidget *m_serialPortDataBitsWidget{};
    QComboBox *m_serialPortDataBitsCombobox{};
    QWidget *m_serialPortParityWidget{};
    QComboBox *m_serialPortParityCombobox{};
    QWidget *m_serialPortStopBitsWidget{};
    QComboBox *m_serialPortStopBitsCombobox{};
    // tcp client
    QWidget *m_tcpClientRemoteAddressWidget{};
    QLineEdit *m_tcpClientRemoteAddressLineEdit{};
    QWidget *m_tcpClientRemotePortWidget{};
    QSpinBox *m_tcpClientRemotePortSpinBox{};
    // tcp server
    QWidget *m_tcpServerLocalAddressWidget{};
    QLineEdit *m_tcpServerLocalAddressLineEdit{};
    QWidget *m_tcpServerLocalPortWidget{};
    QSpinBox *m_tcpServerLocalPortSpinBox{};
    // udp socket
    QWidget *m_udpSocketLocalAddressWidget{};
    QLineEdit *m_udpSocketLocalAddressLineEdit{};
    QWidget *m_udpSocketLocalPortWidget{};
    QSpinBox *m_udpSocketLocalPortSpinBox{};
    QWidget *m_udpSocketRemoteAddressWidget{};
    QLineEdit *m_udpSocketRemoteAddressLineEdit{};
    QWidget *m_udpSocketRemotePortWidget{};
    QSpinBox *m_udpSocketRemotePortSpinBox{};
    // screen & camera
    QWidget *m_screenNameWidget{};
    QComboBox *m_screenNameCombobox{};
    QWidget *m_cameraNameWidget{};
    QComboBox *m_cameraNameCombobox{};
    QWidget *m_areaSelectionWidget{};
    QPushButton *m_areaSelectionPushButton{};
    AreaSelection *m_areaSelectionDialog{};
    // tx/rx
    QWidget *m_txFormatWidget{};
    QComboBox *m_txFormatCombobox{};
    QWidget *m_txSuffixWidget{};
    QComboBox *m_txSuffixCombobox{};
    QWidget *m_rxFormatWidget{};
    QComboBox *m_rxFormatCombobox{};
    // save button
    QPushButton *m_portSettingSavePushButton{};

    enum {
        SERIALPORT = 1,
        TCPCLIENT,
        TCPSERVER,
        UDPSOCKET,
        SCREEN,
        CAMERA
    };
};

#endif //UNICOMM_PORTSETTING_H
