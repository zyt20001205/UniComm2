#ifndef UNICOMM_PORT_H
#define UNICOMM_PORT_H

#include <QJsonArray>
#include <QJsonObject>
#include <QTabWidget>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class PortPage;
class QPushButton;
class QVBoxLayout;
class QTabWidget;

class BasePort;

class PortModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit PortModule();

    ~PortModule() override = default;

    void portConfigSave() const;

    BasePort* currentPort() const;

    void portList(std::vector<std::string> &portList) const;

    void portInsert(int index, QJsonObject portConfig = QJsonObject());

    void portAnnotate() const;

    QHash<QString, BasePort *> m_portHash{};
signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    void portRemove(int index);

    void portReload(int index);

    void portSwap(int srcIndex, int dstIndex);

    void overlayShow() const;

    void overlayHide() const;

    void overlayResize() const;

    QJsonArray m_portConfig{};
    QTabWidget *m_portTabWidget{};
    QWidget *m_portTabOverlay{};
    int m_version = 1;
};

#endif //UNICOMM_PORT_H
