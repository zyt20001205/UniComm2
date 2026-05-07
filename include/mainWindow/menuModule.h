#ifndef UNICOMM_MENUMODULE_H
#define UNICOMM_MENUMODULE_H

#include <QQuickWidget>

class MenuModule final : public QQuickWidget {
    Q_OBJECT

public:
    explicit MenuModule(QWidget *parent = nullptr);

    ~MenuModule() override;

    void propertySet(const QVariantHash &objects);

    void gitInit(bool status) const;

    Q_INVOKABLE void themeSet(int theme);

signals:
    void setTheme(int theme);

private:
    QObject *m_root{};
};

#endif //UNICOMM_MENUMODULE_H
