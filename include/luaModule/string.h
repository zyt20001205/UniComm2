#ifndef UNICOMM_STRING_H
#define UNICOMM_STRING_H

#include <QObject>

class String final : public QObject {
    Q_OBJECT

public:
    explicit String(QObject *parent = nullptr);

    ~String() override = default;

    static std::string toHex(const std::string_view &ba, char separator);

    static std::string fromHex(const std::string &str);
};

#endif //UNICOMM_STRING_H