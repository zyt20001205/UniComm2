#ifndef UNICOMM_PORTMODULE_H
#define UNICOMM_PORTMODULE_H

#include <QJsonObject>
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

    ~PortModule() override;

    void propertySet(const QVariantMap &objects);

    void portConfigSave();

    [[nodiscard]] QSet<QString> portList() const;

    Q_INVOKABLE void portSetting(int index = -1) const;

    void portInsert(int index, const QJsonObject &portConfig = QJsonObject());

    Q_INVOKABLE void portRemove(int index);

    Q_INVOKABLE void portSwap(int src, int dst) const;

    void portEdit(const QString &oldPortName, const QJsonObject &portConfig);

    Q_INVOKABLE void portToggle(int index);

    static void portRefresh(const QString &portName, bool status) ;

    QHash<QString, BasePort *> m_portHash{};
signals:
    void appendLog(int type, const QString &prefix, const QString &message);

private:
    QQuickWidget *m_widget{};
    QQuickItem *m_rootItem{};
    PortSetting *m_portSetting{};
};

#endif //UNICOMM_PORTMODULE_H
