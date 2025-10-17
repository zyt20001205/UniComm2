#ifndef UNICOMM_PORT_H
#define UNICOMM_PORT_H

#include <QJsonArray>
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

    void workspaceOpen(const QUrl &rootUrl);

    void portLoad();

    void portConfigSave() const;

    BasePort* currentPort() const;

    QHash<QString, BasePort *> m_portHash{};
signals:
    void appendLog(const QString &message, const QString &level);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    void portInsert(int index, const QJsonObject &portConfig);

    void portRemove(int index);

    void portSwap(int srcIndex, int dstIndex);

    void overlayShow() const;

    void overlayHide() const;

    void overlayResize() const;

    void portAnnotationRefresh() const;

    QJsonArray m_portConfig{};
    QTabWidget *m_portTabWidget{};
    QWidget *m_portTabOverlay{};
    QUrl m_annotationUrl{};
    int m_version = 1;
};

class PortPage final : public QWidget {
    Q_OBJECT

public:
    explicit PortPage(const QJsonObject &portConfig, QWidget *parent = nullptr);

    ~PortPage() override;

    void portReload(const QJsonObject &portConfig) const;

    BasePort *m_port{};
signals:
    void appendLog(const QString &message, const QString &level);

private:
    void portToggle(bool status) const;

    QPushButton *m_portToggleButton{};
};

#endif //UNICOMM_PORT_H
