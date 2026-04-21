#ifndef UNICOMM_FILE_H
#define UNICOMM_FILE_H

#include <QFile>
#include <QHash>
#include <QObject>
#include <sol/variadic_args.hpp>

namespace sol {
    struct variadic_args;
}

class File final : public QObject {
    Q_OBJECT

public:
    explicit File(QObject *parent = nullptr);

    ~File() override = default;

    void open(const std::string &path, const std::string &mode);

    void close(const std::string &path);

    [[nodiscard]] sol::object read(const std::string &path, const sol::variadic_args &args);

    void write(const std::string &path, const sol::variadic_args &args);

private:
    struct FileHandle {
        QFile file;
        bool binary = false;
    };

    QHash<QUrl, std::shared_ptr<FileHandle> > m_handleHash;
};

#endif //UNICOMM_FILE_H
