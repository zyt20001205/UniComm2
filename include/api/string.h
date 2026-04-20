#ifndef UNICOMM_STRING_H
#define UNICOMM_STRING_H

#include <QObject>

class String final : public QObject {
    Q_OBJECT

public:
    explicit String(QObject *parent = nullptr);

    ~String() override = default;

    [[nodiscard]] static std::string toBase64(const std::string &str);

    [[nodiscard]] static std::string fromBase64(const std::string &str);

    [[nodiscard]] static std::string toHex(const std::string &str, char separator);

    [[nodiscard]] static std::string fromHex(const std::string &str);
};

#endif //UNICOMM_STRING_H