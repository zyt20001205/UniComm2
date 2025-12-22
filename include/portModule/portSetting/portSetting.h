#ifndef UNICOMM_PORTSETTING_H
#define UNICOMM_PORTSETTING_H

#include <QDialog>
#include <QJsonObject>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QStandardItemModel;
class QVBoxLayout;

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

private:
    void serialPortRefresh() const;

    void visaRefresh() const;

    QDialog *m_portSettingDialog{};
    QStandardItemModel *m_serialPortStandardItemModel{};
    QStandardItemModel *m_visaStandardItemModel{};

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
    QObject *m_tcpServerLocalHostTextField{};
    QObject *m_tcpServerLocalPortSpinBox{};
    // udp socket
    QObject *m_udpSocketNameTextField{};
    QObject *m_udpSocketLocalHostTextField{};
    QObject *m_udpSocketLocalPortSpinBox{};
    QObject *m_udpSocketRemoteHostTextField{};
    QObject *m_udpSocketRemotePortSpinBox{};
    // format
    QObject *m_txFormatComboBox{};
    QObject *m_txSuffixComboBox{};
    QObject *m_rxFormatComboBox{};
};

// class PortSetting final : public QDialog {
//     Q_OBJECT
//
// public:
//     explicit PortSetting(const QSet<QString> &portUsedName = QSet<QString>(), QWidget *parent = nullptr);
//
//     ~PortSetting() override = default;
//
//     void portSettingImport(const QJsonObject &portConfig);
//
//     QJsonObject portSettingExport();
//
//     static QStringList visaListGet();
//
// private:
//     void portSettingHideAll();
//
//     void portSettingTypeSwitch(int portType);
//
//     void portSettingSave(int portType);
//
//     // QQuickView* m_portSettingDialog{};
//
//     QSet<QString> m_portUsedName{};
//     QJsonObject m_portConfig{};
//     QVBoxLayout *m_portSettingLayout{};
//     QWidget *m_portTypeWidget{};
//     QComboBox *m_portTypeCombobox{};
//     // serial port
//     QWidget *m_serialPortNameWidget{};
//     QComboBox *m_serialPortNameCombobox{};
//     QWidget *m_serialPortBaudRateWidget{};
//     QSpinBox *m_serialPortBaudRateSpinBox{};
//     QWidget *m_serialPortDataBitsWidget{};
//     QComboBox *m_serialPortDataBitsCombobox{};
//     QWidget *m_serialPortParityWidget{};
//     QComboBox *m_serialPortParityCombobox{};
//     QWidget *m_serialPortStopBitsWidget{};
//     QComboBox *m_serialPortStopBitsCombobox{};
//     // visa
//     QWidget *m_visaNameWidget{};
//     QComboBox *m_visaNameCombobox{};
//     // tcp client
//     QWidget *m_tcpClientNameWidget{};
//     QLineEdit *m_tcpClientNameLineEdit{};
//     QWidget *m_tcpClientRemoteHostWidget{};
//     QLineEdit *m_tcpClientRemoteHostLineEdit{};
//     QWidget *m_tcpClientRemotePortWidget{};
//     QSpinBox *m_tcpClientRemotePortSpinBox{};
//     // tcp server
//     QWidget *m_tcpServerNameWidget{};
//     QLineEdit *m_tcpServerNameLineEdit{};
//     QWidget *m_tcpServerLocalHostWidget{};
//     QLineEdit *m_tcpServerLocalHostLineEdit{};
//     QWidget *m_tcpServerLocalPortWidget{};
//     QSpinBox *m_tcpServerLocalPortSpinBox{};
//     // udp socket
//     QWidget *m_udpSocketNameWidget{};
//     QLineEdit *m_udpSocketNameLineEdit{};
//     QWidget *m_udpSocketLocalHostWidget{};
//     QLineEdit *m_udpSocketLocalHostLineEdit{};
//     QWidget *m_udpSocketLocalPortWidget{};
//     QSpinBox *m_udpSocketLocalPortSpinBox{};
//     QWidget *m_udpSocketRemoteHostWidget{};
//     QLineEdit *m_udpSocketRemoteHostLineEdit{};
//     QWidget *m_udpSocketRemotePortWidget{};
//     QSpinBox *m_udpSocketRemotePortSpinBox{};
//     // screen & camera
//     QWidget *m_screenNameWidget{};
//     QComboBox *m_screenNameCombobox{};
//     QWidget *m_cameraNameWidget{};
//     QComboBox *m_cameraNameCombobox{};
//     QWidget *m_areaSelectionWidget{};
//     QPushButton *m_areaSelectionPushButton{};
//     AreaSelection *m_areaSelectionDialog{};
//     // tx/rx
//     QWidget *m_txFormatWidget{};
//     QComboBox *m_txFormatCombobox{};
//     QWidget *m_txSuffixWidget{};
//     QComboBox *m_txSuffixCombobox{};
//     QWidget *m_rxFormatWidget{};
//     QComboBox *m_rxFormatCombobox{};
//     // save button
//     QPushButton *m_portSettingSavePushButton{};
// };

#endif //UNICOMM_PORTSETTING_H
