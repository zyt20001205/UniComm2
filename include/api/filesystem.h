#ifndef UNICOMM_FILESYSTEM_H
#define UNICOMM_FILESYSTEM_H

#include <QVariantHash>
#include <sol/object.hpp>
#include <sol/optional.hpp>
#include <sol/table.hpp>

#include <memory>
#include <string>

class File;
class QFileInfo;

class Filesystem final {
public:
    [[nodiscard]] static std::shared_ptr<File> open(const std::string &path, const std::string &mode);

    [[nodiscard]] static bool exists(const std::string &path);

    [[nodiscard]] static sol::table list(sol::this_state ts, const sol::optional<std::string> &path);

    [[nodiscard]] static sol::object stat(sol::this_state ts, const std::string &path);

    static void copy(const std::string &from, const std::string &to);

    static void mkdir(const std::string &path);

    static void remove(const std::string &path);

    static void rename(const std::string &from, const std::string &to);

    static void rmdir(const std::string &path);

    static void openExternal(const std::string &path);

private:
    [[nodiscard]] static QVariantHash fileInfo(const QFileInfo &info);

    [[nodiscard]] static QString resolve(const std::string &path);

    static void ensureMutable(const QString &path);
};

#endif //UNICOMM_FILESYSTEM_H
