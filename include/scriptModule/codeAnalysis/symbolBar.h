#ifndef UNICOMM_SYMBOLBAR_H
#define UNICOMM_SYMBOLBAR_H

#include <QQuickWidget>

class SymbolBar final : public QQuickWidget {
    Q_OBJECT

public:
    explicit SymbolBar(QWidget *parent = nullptr);

    void propertySet(const QVariantMap &objects);

private:
};

#endif //UNICOMM_SYMBOLBAR_H