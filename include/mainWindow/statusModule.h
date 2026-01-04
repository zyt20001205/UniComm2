#ifndef UNICOMM_STATUSMODULE_H
#define UNICOMM_STATUSMODULE_H

#include <QQuickWidget>

class StatusModule final : public QQuickWidget {
    Q_OBJECT

public:
    explicit StatusModule(QWidget *parent = nullptr);

    ~StatusModule() override;

    void propertySet(const QVariantMap &objects);
};

#endif //UNICOMM_STATUSMODULE_H