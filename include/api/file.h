#ifndef UNICOMM_FILE_H
#define UNICOMM_FILE_H

#include <string>

class FileSystem final {
public:
    static void open(const std::string &path, const std::string &mode);

    static void close(const std::string &path);

    static std::string read(const std::string &path);

    static void write(const std::string &path, const std::string &data);
};

#endif //UNICOMM_FILE_H
