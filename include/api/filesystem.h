#ifndef UNICOMM_FILESYSTEM_H
#define UNICOMM_FILESYSTEM_H

#include <memory>
#include <string>

class File;

class Filesystem final {
public:
    [[nodiscard]] static std::shared_ptr<File> open(const std::string &path, const std::string &mode);

    static void remove(const std::string &path);

    static void rename(const std::string &from, const std::string &to);
};

#endif //UNICOMM_FILESYSTEM_H
