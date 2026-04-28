#ifndef UNICOMM_MENUMODULE_H
#define UNICOMM_MENUMODULE_H

#include <QQuickWidget>

class MenuModule final : public QQuickWidget {
    Q_OBJECT

public:
    explicit MenuModule(QWidget *parent = nullptr);

    ~MenuModule() override;

    void propertySet(const QVariantMap &objects);

    void gitInit(bool status) const;

private:
    QObject *m_root{};
};

#endif //UNICOMM_MENUMODULE_H