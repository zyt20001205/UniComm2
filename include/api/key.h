#ifndef UNICOMM_KEY_H
#define UNICOMM_KEY_H

#include <QHash>
#include <QObject>

class Key final : public QObject {
    Q_OBJECT

public:
    explicit Key(QObject *parent = nullptr);

    ~Key() override = default;

    void tap(const std::string &key);

    static void type(const std::string &text);

private:
    QHash<QString, int> m_vkHash{};
};

#endif //UNICOMM_KEY_H