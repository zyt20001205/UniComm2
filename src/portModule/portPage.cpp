#include "portModule/portPage.h"

#include <QPushButton>
#include <QVBoxLayout>

#include "globals.h"
#include "portModule/camera.h"
#include "portModule/pixmapPreview.h"
#include "portModule/screen.h"
#include "portModule/serialPort.h"
#include "portModule/tcpClient.h"
#include "portModule/tcpServer.h"
#include "portModule/udpSocket.h"

// PortPage public
PortPage::PortPage(const QJsonObject &portConfig, QWidget *parent)
    : QWidget(parent),
      m_portToggleButton(new QPushButton(tr("Open"))),
      m_pixmapPreview(new PixmapPreview(this)) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    m_portToggleButton->setCheckable(true);

    QString timestamp;
    switch (portConfig["portType"].toInt()) {
        case SERIALPORT: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new SerialPort(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            break;
        }
        case TCPCLIENT: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new TcpClient(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            break;
        }
        case TCPSERVER: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new TcpServer(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            break;
        }
        case UDPSOCKET: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new UdpSocket(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
            });
            break;
        }
        case SCREEN: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new Screen(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
                m_pixmapPreview->setVisible(status);
            });
            connect(m_port, &BasePort::showPreview, m_pixmapPreview, &PixmapPreview::previewShow);
            break;
        }
        case CAMERA: {
            layout->addWidget(m_portToggleButton);
            connect(m_portToggleButton, &QPushButton::clicked, this, &PortPage::portToggle);

            m_port = new Camera(portConfig);
            connect(m_port, &BasePort::appendLog, this, &PortPage::appendLog);
            connect(m_port, &BasePort::togglePort, this, [this](const bool status) {
                m_portToggleButton->setChecked(status);
                m_pixmapPreview->setVisible(status);
            });
            connect(m_port, &BasePort::showPreview, m_pixmapPreview, &PixmapPreview::previewShow);
            break;
        }
        default: {
            qDebug() << "unknown port type";
            break;
        }
    }
}

PortPage::~PortPage() {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    delete m_port;
}

void PortPage::portReload(const QJsonObject &portConfig) const {
    QMetaObject::invokeMethod(m_port, [this] {
        m_port->close();
    }, Qt::BlockingQueuedConnection);
    m_portToggleButton->setChecked(false);
    QMetaObject::invokeMethod(m_port, [this, portConfig] {
        m_port->reload(portConfig);
    }, Qt::BlockingQueuedConnection);
}

// PortPage private
void PortPage::portToggle(const bool status) const {
    if (status) {
        bool ok = false;
        QMetaObject::invokeMethod(m_port, [&ok, this] {
            ok = m_port->open();
        }, Qt::BlockingQueuedConnection);
        m_portToggleButton->setChecked(ok);
    } else {
        QMetaObject::invokeMethod(m_port, [this] {
            m_port->close();
        }, Qt::BlockingQueuedConnection);
        m_portToggleButton->setChecked(false);
    }
}
