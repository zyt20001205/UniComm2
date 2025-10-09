#ifndef PORT_H
#define PORT_H

#include <QApplication>
#include <QButtonGroup>
#include <QCamera>
#include <QCameraDevice>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDockWidget>
#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHeaderView>
#include <QImage>
#include <QImageCapture>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSpinbox>
#include <QSplitter>
#include <QStackedLayout>
#include <QTabWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QWidget>
#include <QMutex>
#include <allheaders.h>
#include <baseapi.h>
#include <QStackedWidget>
#include <QStandardItemModel>

class AreaSelectDialog;

class BasePort;

class PortModule final : public QDockWidget {
    Q_OBJECT

public:
    explicit PortModule(QWidget *parent = nullptr);

    ~PortModule() override = default;

    void portConfigSave() const;

    BasePort *portObject(int index) const;

signals:
    void appendLog(const QString &message, const QString &level);

private:
    // port widget
    void portMenu(int index, const QPoint &pos);

    void portSelected(int index);

    void portDuplicate(int index);

    void portRemove(int index);

    void portSwap(int srcIndex, int dstIndex);

    void previewShow(const QList<QPixmap>& pixmapList) const;

    QJsonArray m_portConfig{};
    QTabWidget *m_tabWidget{};
    QDialog *m_previewDialog{};
    QVBoxLayout *m_previewLayout{};
    int m_currentIndex = 0;

    // port setting dialog
    void portSettingLoad(int index);

    void portSettingWidgetReset() const;

    void portSettingTypeSwitch(int type);

    void portSettingSave(int type);

    QDialog *m_portSettingDialog = nullptr;
    QVBoxLayout *m_portSettingLayout = nullptr;
    QWidget *m_portTypeWidget = nullptr;
    QComboBox *m_portTypeCombobox = nullptr;
    // serial port
    QWidget *m_serialPortNameWidget = nullptr;
    QComboBox *m_serialPortNameCombobox = nullptr;
    QWidget *m_serialPortBaudRateWidget = nullptr;
    QSpinBox *m_serialPortBaudRateSpinBox = nullptr;
    QWidget *m_serialPortDataBitsWidget = nullptr;
    QComboBox *m_serialPortDataBitsCombobox = nullptr;
    QWidget *m_serialPortParityWidget = nullptr;
    QComboBox *m_serialPortParityCombobox = nullptr;
    QWidget *m_serialPortStopBitsWidget = nullptr;
    QComboBox *m_serialPortStopBitsCombobox = nullptr;
    // tcp client
    QWidget *m_tcpClientRemoteAddressWidget = nullptr;
    QLineEdit *m_tcpClientRemoteAddressLineEdit = nullptr;
    QWidget *m_tcpClientRemotePortWidget = nullptr;
    QSpinBox *m_tcpClientRemotePortSpinBox = nullptr;
    // tcp server
    QWidget *m_tcpServerLocalAddressWidget = nullptr;
    QLineEdit *m_tcpServerLocalAddressLineEdit = nullptr;
    QWidget *m_tcpServerLocalPortWidget = nullptr;
    QSpinBox *m_tcpServerLocalPortSpinBox = nullptr;
    // udp socket
    QWidget *m_udpSocketLocalAddressWidget = nullptr;
    QLineEdit *m_udpSocketLocalAddressLineEdit = nullptr;
    QWidget *m_udpSocketLocalPortWidget = nullptr;
    QSpinBox *m_udpSocketLocalPortSpinBox = nullptr;
    QWidget *m_udpSocketRemoteAddressWidget = nullptr;
    QLineEdit *m_udpSocketRemoteAddressLineEdit = nullptr;
    QWidget *m_udpSocketRemotePortWidget = nullptr;
    QSpinBox *m_udpSocketRemotePortSpinBox = nullptr;
    // screen/camera
    QWidget *m_screenNameWidget = nullptr;
    QComboBox *m_screenNameCombobox = nullptr;
    QWidget *m_cameraNameWidget = nullptr;
    QComboBox *m_cameraNameCombobox = nullptr;
    QWidget *m_areaSelectWidget = nullptr;
    QPushButton *m_areaSelectPushButton = nullptr;
    AreaSelectDialog *m_areaChooseDialog = nullptr;
    // tx/rx
    QWidget *m_txFormatWidget = nullptr;
    QComboBox *m_txFormatCombobox = nullptr;
    QWidget *m_txSuffixWidget = nullptr;
    QComboBox *m_txSuffixCombobox = nullptr;
    QWidget *m_rxFormatWidget = nullptr;
    QComboBox *m_rxFormatCombobox = nullptr;
    // save button
    QPushButton *m_portSettingSavePushButton = nullptr;
};

class PageWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PageWidget(const QJsonObject &portConfig, QObject *parent = nullptr);

    ~PageWidget() override;

    void portReload(const QJsonObject &portConfig) const;

    BasePort *m_port = nullptr;
signals:
    void appendLog(const QString &message, const QString &level);

private:
    void portToggle(bool status) const;

    QPushButton *m_pushButton = nullptr;
    QThread *m_thread = nullptr;
};

#endif //PORT_H
