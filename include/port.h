#ifndef PORT_H
#define PORT_H

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDockWidget>
#include <QFile>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSpinbox>
#include <QTabWidget>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <baseapi.h>
#include <allheaders.h>
#include "config.h"
#include "suffix.h"

class BasePort;

class Port final : public QDockWidget {
    Q_OBJECT

public:
    explicit Port(QObject *parent = nullptr);

    ~Port() override = default;

    void portOpen(int index) const;

    void portClose(int index) const;

    void portWrite(const QString &command, int index) const;

    QString portRead(int index) const;

public slots:
    void portConfigSave() const;

    // port widget
private:
    void uiInit();

    void portMenu(int index, const QPoint &pos);

    void portSelected(int index);

    void portRemove(int index);

    QJsonArray m_portConfig = g_config["portConfig"].toArray();
    int m_currentIndex = 0;
    QTabWidget *m_tabWidget = nullptr;
    QPushButton *m_addButton = nullptr;

    // port setting dialog
private:
    void portSettingUiInit();

    void portSettingLoad(int index);

    void portSettingWidgetReset() const;

private slots:
    void portSettingTypeSwitch(int type);

    void portSettingSave(int type);

private:
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
    QWidget *m_tcpServerAddressWidget = nullptr;
    QLineEdit *m_tcpServerAddressLineEdit = nullptr;
    QWidget *m_tcpServerPortWidget = nullptr;
    QSpinBox *m_tcpServerPortSpinBox = nullptr;

    // tcp server
    QWidget *m_tcpClientAddressWidget = nullptr;
    QLineEdit *m_tcpClientAddressLineEdit = nullptr;
    QWidget *m_tcpClientPortWidget = nullptr;
    QSpinBox *m_tcpClientPortSpinBox = nullptr;

    // udp socket

    // camera
    QWidget *m_cameraNameWidget = nullptr;
    QComboBox *m_cameraNameCombobox = nullptr;
    QWidget *m_cameraAreaWidget = nullptr;
    QPushButton *m_cameraAreaPushButton = nullptr;
    QDialog *m_cameraAreaChooseDialog = nullptr;
    QGraphicsView *m_cameraAreaChooseGraphicsView = nullptr;

    // tx/rx
    QWidget *m_txFormatWidget = nullptr;
    QComboBox *m_txFormatCombobox = nullptr;
    QWidget *m_txSuffixWidget = nullptr;
    QComboBox *m_txSuffixCombobox = nullptr;
    QWidget *m_txIntervalWidget = nullptr;
    QSpinBox *m_txIntervalSpinBox = nullptr;
    QWidget *m_rxFormatWidget = nullptr;
    QComboBox *m_rxFormatCombobox = nullptr;
    QWidget *m_rxTimeoutWidget = nullptr;
    QSpinBox *m_rxTimeoutSpinBox = nullptr;

    // save button
    QPushButton *m_portSettingSavePushButton = nullptr;

signals:
    void appendLog(const QString &message, const QString &level);
};

class PageWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PageWidget(QObject *parent = nullptr);

    ~PageWidget() override = default;

    void uiInit(const QJsonObject &portConfig);

    void portOpen() const;

    void portClose() const;

    void portWrite(const QString &command) const;

    QString portRead() const;

    void portReload(const QJsonObject &portConfig) const;

private:
    QPushButton *m_pushButton = nullptr;
    BasePort *m_port = nullptr;

private slots:
    void portToggle(bool status);

signals:
    void appendLog(const QString &message, const QString &level);
};

class BasePort : public QObject {
    Q_OBJECT

public:
    explicit BasePort(QObject *parent = nullptr) : QObject(parent) {
    }

    virtual void reload(const QJsonObject &portConfig) =0;

    virtual bool open() = 0;

    virtual void close() = 0;

    virtual void write(const QString &content) = 0;

    virtual QString read() = 0;

signals:
    void appendLog(const QString &message, const QString &level);
};

class SerialPort final : public BasePort {
    Q_OBJECT

public:
    explicit SerialPort(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    void write(const QString &command) override;

    QString read() override;

private:
    void handleWrite();

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
    int m_txInterval;
    // rx config
    QString m_rxFormat;
    int m_rxTimeout;
    QString m_rxForward;
    //
    QList<QByteArray> m_txQueue;
    bool m_txBlock = false;

    QString m_rxBuffer;

private slots:
    void handleRead();

    void handleError();

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);
};

class TcpClient final : public BasePort {
    Q_OBJECT

public:
    explicit TcpClient(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    void write(const QString &command) override;

    QString read() override;

private:
    void handleWrite();

    QTcpSocket *m_tcpClient;
    // port config
    QString m_portName;
    QString m_tcpClientAddress;
    int m_tcpClientPort;
    QString m_tcpServerAddress;
    int m_tcpServerPort;
    // tx config
    QString m_txFormat;
    QString m_txSuffix;
    int m_txInterval;
    // rx config
    QString m_rxFormat;
    int m_rxTimeout;
    QString m_rxForward;
    //
    QList<QByteArray> m_txQueue;
    bool m_txBlock = false;

    QString m_rxBuffer;

private slots:
    void handleConnected();

    void handleDisconnected();

    void handleRead();

    void handleError();

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);
};

class Camera final : public BasePort {
    Q_OBJECT

public:
    explicit Camera(const QJsonObject &portConfig, QObject *parent = nullptr);

    void reload(const QJsonObject &portConfig) override;

    bool open() override;

    void close() override;

    void write(const QString &command) override;

    QString read() override;

private:
    QObject *m_camera;

    // port config
    QString m_portName;
};

#endif //PORT_H
