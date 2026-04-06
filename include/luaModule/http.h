#ifndef UNICOMM_HTTP_H
#define UNICOMM_HTTP_H

#include <QObject>

class Http final : public QObject {
    Q_OBJECT

public:
    explicit Http(QObject *parent = nullptr);

    ~Http() override = default;

    static void get(const std::string &portName);
};

#endif //UNICOMM_HTTP_H
