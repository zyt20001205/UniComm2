#ifndef UNICOMM_MOUSE_H
#define UNICOMM_MOUSE_H

#include <QObject>

class Mouse final : public QObject {
    Q_OBJECT

public:
    explicit Mouse(QObject *parent = nullptr);

    ~Mouse() override = default;

    static void click(int x, int y);

    static void doubleClick(int x, int y);

    static void rightClick(int x, int y);
};

#endif //UNICOMM_MOUSE_H