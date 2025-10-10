#ifndef UNICOMM_PORT_H
#define UNICOMM_PORT_H

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

#endif //UNICOMM_PORT_H
