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
class PortPage;

class PortModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit PortModule();

    ~PortModule() override = default;

    void propertySet(const QVariantMap &objects);

    void portConfigSave() const;

    BasePort* currentPort() const;

    void portList(std::vector<std::string> &portList) const;

    Q_INVOKABLE void portInsert(int index, QJsonObject portConfig = QJsonObject());

    Q_INVOKABLE void portRemove(int index);

    void portAnnotate() const;

    Q_INVOKABLE void portToggle(const QString &portName, bool status);

    QHash<QString, BasePort *> m_portHash{};
signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void portReload(int index);

    void portSwap(int srcIndex, int dstIndex);

    QJsonArray m_portConfig{};
    QQuickWidget *m_portWidget{};
    QQuickItem *m_portRoot{};
    QStandardItemModel *m_portStandardItemModel{};

    QTabWidget *m_portTabWidget{};
    int m_version = 1;
};

#endif //UNICOMM_PORT_H
