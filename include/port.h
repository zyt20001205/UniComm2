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
#include <QList>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSpinbox>
#include <QTabWidget>
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

    void portWrite(const QString &command, int index);

    QString portRead(int index);

    // port widget
private:
    void uiInit();

    void portToggle(bool status);

    void portMenu(int index, const QPoint &pos);

    void portSelected(int index);

    void portRemove(int index);

    QJsonArray m_portConfig = g_config["portConfig"].toArray();
    int m_currentIndex = 0;
    QList<BasePort *> m_portList;
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

    // tcp server

    // udp socket

    // camera
    QWidget *m_cameraNameWidget = nullptr;
    QComboBox *m_cameraNameCombobox = nullptr;
    QWidget *m_cameraAreaWidget = nullptr;
    QPushButton *m_cameraAreaPushButton = nullptr;
    QDialog *m_cameraAreaChooseDialog = nullptr;
    QGraphicsView *m_cameraAreaChooseGraphicsView = nullptr;

    // tx/rx
    QWidget *m_rxTimeoutWidget = nullptr;
    QSpinBox *m_rxTimeoutSpinBox = nullptr;

    // save button
    QPushButton *m_portSettingSavePushButton = nullptr;

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

signals:
    void connected();

    void disconnected();

    void readyRead();

    void errorOccurred(const QString &error);

    void appendLog(const QString &message, const QString &level);

private slots:
    void handleRead();

    void handleError();

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

    QString m_txBuffer;
    QString m_rxBuffer;
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
