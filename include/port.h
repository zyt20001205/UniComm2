#ifndef PORT_H
#define PORT_H

#include <QApplication>
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
#include <QStandardItemModel>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "config.h"
#include "suffix.h"
#include "utils.h"

class AreaSelectDialog;

class BasePort;

class Port final : public QDockWidget {
    Q_OBJECT

public:
    explicit Port(QObject *parent = nullptr);

    ~Port() override = default;

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

    void portSwap(const int srcIndex, const int dstIndex);

    QJsonArray m_portConfig = g_config["portConfig"].toArray();
    QTabWidget *m_tabWidget = nullptr;
    int m_currentIndex = 0;
    QPushButton *m_addButton = nullptr;

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

class AreaSelectDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AreaSelectDialog(QWidget *parent = nullptr);

    ~AreaSelectDialog() override = default;

    void reload(const QJsonObject &config) const;

    void capture(const QString &type, const QString &target);

    QString charsetExport() const;

    QJsonArray areaExport() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void select(bool status) const;

    void charsetRefresh();

    void selectionHandle(const QRectF &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);

    void selectionRefresh() const;

    void ocrRefresh() const;

    QString m_type;
    QString m_target;
    qreal m_dpr{};
    QGraphicsView *m_graphicsView = nullptr;
    QString m_charsetString= "eng";
    QListView *m_charsetListView = nullptr;
    QStandardItemModel *m_charsetModel = nullptr;
    QListView *m_selectionListView = nullptr;
    QStandardItemModel *m_selectionModel = nullptr;
    int m_process = 0;
    QPixmap m_shot{};
    QRectF m_rectF;

    enum {
        RAW,
        THRESHOLD,
    };
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

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr) : QObject(parent) {
    }

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
};

class SerialPort final : public BasePort {
    Q_OBJECT

public:
    explicit SerialPort(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QHash<QString, QVariant> info() override;

    bool open() override;

    void close() override;

    void writeText(const QString &txText) override;

    void writeData(const QByteArray &txData) override;

    QString readText(int timeout, int length) override;

    QByteArray readData(int timeout, int length) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    void handleError();

    QSerialPort *m_serialPort;
    // port config
    QString m_portName;
    int m_baudRate;
    int m_dataBits;
    int m_parity;
    int m_stopBits;
    // tx config
    QString m_txFormat;
    QString m_txSuffix;
    // rx config
    QString m_rxFormat;
    //
    QByteArray m_rxBuffer;
};

class TcpClient final : public BasePort {
    Q_OBJECT

public:
    explicit TcpClient(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QHash<QString, QVariant> info() override;

    bool open() override;

    void close() override;

    void writeText(const QString &txText) override;

    void writeData(const QByteArray &txData) override;

    QString readText(int timeout, int length) override;

    QByteArray readData(int timeout, int length) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleConnected();

    void handleDisconnected();

    void handleError();

    void handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    QTcpSocket *m_tcpClient;
    // port config
    QString m_portName;
    QString m_tcpClientRemoteAddress;
    int m_tcpClientRemotePort;
    QString m_tcpClientLocalAddress;
    int m_tcpClientLocalPort{};
    // tx config
    QString m_txFormat;
    QString m_txSuffix;
    // rx config
    QString m_rxFormat;
    //
    QByteArray m_rxBuffer;
};

class TcpServer final : public BasePort {
    Q_OBJECT

public:
    explicit TcpServer(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    QHash<QString, QVariant> info() override;

    void writeText(const QString &txText) override;

    void writeText(const QString &txText, const QString &peerIp) override;

    void writeData(const QByteArray &txData) override;

    void writeData(const QByteArray &txData, const QString &peerIp) override;

    QString readText(int timeout, int length, const QString &peerIp) override;

    QByteArray readData(int timeout, int length, const QString &peerIp) override;

signals:
    void newConnection();

    void acceptError(const QString &error);

    void disconnected(qintptr socketDescriptor);

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleNewConnection();

    void handleServerError();

    void handleConnected(QTcpSocket *tcpServerPeer);

    void handleDisconnected(QTcpSocket *tcpServerPeer);

    void handleError(QTcpSocket *tcpServerPeer);

    void handleWrite(const QByteArray &f_txData, const QString &peerIp = QString());

    QByteArray handleRead(int timeout, int length, QTcpSocket *tcpServerPeer);

    QTcpServer *m_tcpServer;
    // port config
    QString m_portName;
    QString m_tcpServerLocalAddress;
    int m_tcpServerLocalPort;
    QList<QTcpSocket *> m_tcpServerPeerList;
    // tx config
    QString m_txFormat;
    QString m_txSuffix;
    // rx config
    QString m_rxFormat;
    //
    QByteArray m_rxBuffer;
};

class UdpSocket final : public BasePort {
    Q_OBJECT

public:
    explicit UdpSocket(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    QHash<QString, QVariant> info() override;

    bool open() override;

    void close() override;

    void writeText(const QString &txText) override;

    void writeData(const QByteArray &txData) override;

    QString readText(int timeout, int length) override;

    QByteArray readData(int timeout, int length) override;

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

private:
    void handleError();

    void handleWrite(const QByteArray &f_txData);

    QByteArray handleRead(int timeout, int length);

    QUdpSocket *m_udpSocket;
    // port config
    QString m_portName;
    QString m_udpSocketLocalAddress;
    int m_udpSocketLocalPort;
    QString m_udpSocketRemoteAddress;
    int m_udpSocketRemotePort;
    // tx config
    QString m_txFormat;
    QString m_txSuffix;
    // rx config
    QString m_rxFormat;
    //
    QByteArray m_rxBuffer;
};

class Screen final : public BasePort {
    Q_OBJECT

public:
    explicit Screen(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    QHash<QString, QVariant> info() override;

    QString readText(int timeout, int length) override;

private:
    QScreen *m_screen = nullptr;
    // port config
    QString m_portName;
    QString m_charset;
    QJsonArray m_areaList;
};

class Camera final : public BasePort {
    Q_OBJECT

public:
    explicit Camera(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    QHash<QString, QVariant> info() override;

    QString readText(int timeout, int length) override;

private:
    QCameraDevice m_camera;
    // port config
    QString m_portName;
    QString m_charset;
    QJsonArray m_areaList;
};

#endif //PORT_H
