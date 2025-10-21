#include "portModule/portModule.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "portModule/portPage.h"
#include "portModule/portSetting.h"

// PortModule public
PortModule::PortModule()
    : DockWidget("port"),
      m_portTabWidget(new QTabWidget()),
      m_portTabOverlay(new QWidget(m_portTabWidget)) {
    setMinimumHeight(100);
    setWidget(m_portTabWidget);
    m_portTabWidget->setTabsClosable(true);
    m_portTabWidget->setMovable(true);
    connect(m_portTabWidget, &QTabWidget::tabCloseRequested, this, &PortModule::portRemove);
    connect(m_portTabWidget->tabBar(), &QTabBar::tabMoved, this, &PortModule::portSwap);
    auto *addButton = new QPushButton(); // NOLINT
    addButton->setFixedSize(24, 24);
    addButton->setIcon(QIcon(":/icon/add.svg"));
    m_portTabWidget->setCornerWidget(addButton, Qt::TopRightCorner);
    connect(addButton, &QPushButton::clicked, this, [this] { portInsert(-1); });

    m_portTabOverlay->installEventFilter(this);
    m_portTabOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 96);");
    auto *overlayLayout = new QVBoxLayout(m_portTabOverlay); // NOLINT
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    auto *overlayLabel = new QLabel(tr("Click to add a port")); // NOLINT
    overlayLayout->addWidget(overlayLabel);
    overlayLabel->setFont(QFont("Consolas", 12, QFont::Bold));
    overlayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 0); color: white;");
    if (m_portTabWidget->count() == 0) overlayShow();
}

void PortModule::workspaceOpen(const QUrl &rootUrl) {
    if (m_annotationUrl.isEmpty()) {
        // post initialization after workspace opened
        for (const auto &value: g_config["portConfig"].toArray()) {
            const QJsonObject portConfig = value.toObject();
            portInsert(-1, portConfig);
        }
    } else {
        // nothing to do here
    }
    const QString rootPath = rootUrl.toLocalFile();
    const QString annotationPath = QDir(rootPath).filePath("lib/port.d.lua");
    m_annotationUrl = QUrl::fromLocalFile(annotationPath);
    portAnnotate();
}

void PortModule::portConfigSave() const {
    g_config["portConfig"] = m_portConfig;
}

BasePort *PortModule::currentPort() const {
    if (m_portTabWidget->currentWidget()) {
        return static_cast<PortPage *>(m_portTabWidget->currentWidget())->m_port;
    }
    return nullptr;
}

QVariantList PortModule::portList() const {
    QVariantList portList{};
    for (const QString &portName: m_portHash.keys()) {
        portList.append(portName);
    }
    return portList;
}

void PortModule::portInsert(int index, QJsonObject portConfig) {
    if (index == -1) {
        index = m_portConfig.size();
    }
    if (portConfig.isEmpty()) {
        const QSet usedPortName(m_portHash.keyBegin(), m_portHash.keyEnd());
        if (PortSetting portSettingDialog(usedPortName); portSettingDialog.exec() == QDialog::Accepted) {
            portConfig = portSettingDialog.portSettingExport();
        } else {
            return;
        }
    }
    const QString portName = portConfig["portName"].toString();
    // frontend
    auto *portPage = new PortPage(portConfig); // NOLINT
    connect(portPage, &PortPage::appendLog, this, &PortModule::appendLog);
    /* connect(pageWidget->m_port, &BasePort::showPreview, this, &PortModule::previewShow);*/
    m_portTabWidget->insertTab(index, portPage, portName);
    m_portTabWidget->setCurrentWidget(portPage);
    overlayHide();
    // backend
    m_portConfig.insert(index, portConfig);
    m_portHash.insert(portName, portPage->m_port);
    if (!m_annotationUrl.isEmpty()) portAnnotate();
    // logging
    emit appendLog(QString("%1 initialized").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 initialized").arg(timestamp, portName);
}

// PortModule protected
void PortModule::contextMenuEvent(QContextMenuEvent *event) {
    const QPoint globalPos = event->globalPos();
    const auto *tabBar = m_portTabWidget->tabBar();
    const QPoint tabBarPos = tabBar->mapFromGlobal(globalPos);
    if (tabBar->rect().contains(tabBarPos)) {
        const int index = tabBar->tabAt(tabBarPos);
        m_portTabWidget->setCurrentIndex(index);
        QMenu menu(this);
        menu.addAction("edit", [this, index] { portReload(index); });
        menu.exec(event->globalPos());
    }
}

bool PortModule::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_portTabOverlay && event->type() == QEvent::MouseButtonPress) {
        portInsert(-1);
        return true;
    }
    return DockWidget::eventFilter(obj, event);
}

void PortModule::resizeEvent(QResizeEvent *event) {
    DockWidget::resizeEvent(event);
    if (!m_portTabOverlay->isHidden()) overlayResize();
}

// PortModule private
void PortModule::portRemove(const int index) {
    QJsonObject portConfig = m_portConfig[index].toObject();
    QString portName = portConfig["portName"].toString();
    const QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr,
        tr("Remove Port"),
        QString(tr("Are you sure to remove port %1?")).arg(portName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    // frontend
    QWidget *w = m_portTabWidget->widget(index);
    m_portTabWidget->removeTab(index);
    if (w) w->deleteLater();
    if (m_portTabWidget->count() == 0) overlayShow();
    // backend
    m_portConfig.removeAt(index);
    m_portHash.remove(portName);
    portAnnotate();
    // logging
    emit appendLog(QString("%1 removed").arg(portName), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 removed").arg(timestamp, portName);
}

void PortModule::portReload(const int index) {
    const auto *portPage = static_cast<PortPage *>(m_portTabWidget->widget(index));
    PortSetting portSettingDialog{};
    const QJsonObject oldPortConfig = m_portConfig[index].toObject();
    const QString oldPortName = oldPortConfig["portName"].toString();
    portSettingDialog.portSettingImport(oldPortConfig);
    if (portSettingDialog.exec() == QDialog::Accepted) {
        const QJsonObject newPortConfig = portSettingDialog.portSettingExport();
        const QString newPortName = newPortConfig["portName"].toString();
        if (newPortName != oldPortName) {
            // frontend
            m_portTabWidget->setTabText(index, newPortName);
            // backend
            BasePort *port = m_portHash.value(oldPortName);
            m_portHash.remove(oldPortName);
            m_portHash.insert(newPortName, port);
            portAnnotate();
        }
        m_portConfig[index] = newPortConfig;
        portPage->portReload(newPortConfig);
        // logging
        emit appendLog(QString("%1 reloaded").arg(oldPortName), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 reloaded").arg(timestamp, oldPortName);
    }
}

void PortModule::portSwap(const int srcIndex, const int dstIndex) {
    // config
    const QJsonValue tmp = m_portConfig.takeAt(srcIndex);
    m_portConfig.insert(dstIndex, tmp);
}

void PortModule::portAnnotate() const {
    QString annotation;
    annotation += "--- @meta\n\n";
    annotation += "--- @alias port\n";
    for (const QString &portName: m_portHash.keys()) {
        const QString escapedName = QString(portName).replace("\\", "\\\\");
        annotation += QString("--- | '\"%1\"'\n").arg(escapedName);
    }
    annotation += QString("--- | '\"Add New Port\"'\n");
    annotation += "\n";

    QFile file(m_annotationUrl.toLocalFile());
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << annotation;
    file.close();
}

void PortModule::overlayShow() const {
    overlayResize();
    m_portTabOverlay->raise();
    m_portTabOverlay->show();
}

void PortModule::overlayHide() const {
    m_portTabOverlay->hide();
}

void PortModule::overlayResize() const {
    m_portTabOverlay->resize(m_portTabWidget->size());
    m_portTabOverlay->move(0, 0);
}
