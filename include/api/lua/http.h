#ifndef UNICOMM_HTTP_H
#define UNICOMM_HTTP_H

#include <QObject>
#include <sol/table.hpp>

class Http final : public QObject {
    Q_OBJECT

public:
    explicit Http(QObject *parent = nullptr);

    ~Http() override = default;

    static void get(const std::string &portName, const sol::table &headers, int timeout);
};

#endif //UNICOMM_HTTP_H
