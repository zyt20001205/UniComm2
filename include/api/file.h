#ifndef UNICOMM_FILE_H
#define UNICOMM_FILE_H

#include <QFile>
#include <sol/object.hpp>

#include <string>

class File final {
public:
    File(const QString &path, const std::string &mode);

    ~File() = default;

    bool close();

    bool flush();

    [[nodiscard]] sol::object read(const sol::variadic_args &args);

    [[nodiscard]] qint64 seek(const sol::optional<std::string> &whence, const sol::optional<qint64> &offset);

    [[nodiscard]] File *write(const sol::variadic_args &args);

private:
    QFile m_file{};
    bool m_binary{};
};

#endif //UNICOMM_FILE_H
