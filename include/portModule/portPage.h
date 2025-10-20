#ifndef UNICOMM_PORTPAGE_H
#define UNICOMM_PORTPAGE_H

#include <QWidget>

class QPushButton;

class BasePort;
class PixmapPreview;

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
    PixmapPreview *m_pixmapPreview{};
};

#endif //UNICOMM_PORTPAGE_H
