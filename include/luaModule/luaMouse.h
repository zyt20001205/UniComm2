#ifndef UNICOMM_LUAMOUSE_H
#define UNICOMM_LUAMOUSE_H

#include <QObject>

class LuaMouse final : public QObject {
    Q_OBJECT

public:
    explicit LuaMouse(QObject *parent = nullptr);

    ~LuaMouse() override = default;

    static void click(int x, int y);

    static void doubleClick(int x, int y);

    static void rightClick(int x, int y);
};

#endif //UNICOMM_LUAMOUSE_H