#ifndef UNICOMM_PORT_H
#define UNICOMM_PORT_H

#include <QJsonArray>
#include <QJsonObject>
#include <QTabWidget>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QPushButton;
class QQuickWidget;
class QStandardItemModel;
class QTabWidget;
class QVBoxLayout;

class BasePort;
class PortSetting;

class PortModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit PortModule();

    ~PortModule() override = default;

    void propertySet(const QVariantMap &objects);

    void portConfigSave();

    BasePort *currentPort() const;

    void portList(QSet<QString> &portList) const;

    Q_INVOKABLE void portSetting(const QString &portName = QString()) const;

    void portInsert(int index, const QJsonObject &portConfig = QJsonObject());

    Q_INVOKABLE void portRemove(const QString &portName);

    void portEdit(const QString &oldPortName, const QJsonObject &portConfig);

    Q_INVOKABLE void portToggle(const QString &portName);

    void portRefresh(const QString &portName, bool status) const;

    QHash<QString, BasePort *> m_portHash{};
signals:
    void appendLog(const QString &message, const QString &level);

private:
    QQuickWidget *m_portWidget{};
    QQuickItem *m_portRoot{};
    QStandardItemModel *m_portStandardItemModel{};
    PortSetting *m_portSetting{};

    int m_version = 1;
};

#endif //UNICOMM_PORT_H
