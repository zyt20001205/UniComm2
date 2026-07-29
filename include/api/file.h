#ifndef UNICOMM_FILE_H
#define UNICOMM_FILE_H

#include <QFile>
#include <sol/object.hpp>
#include <sol/optional.hpp>
#include <sol/variadic_args.hpp>

#include <memory>
#include <string>

class QTextStream;

class File final {
public:
    File(const QString &path, const std::string &mode);

    ~File();

    [[nodiscard]] bool atEnd() const;

    bool close();

    bool flush();

    [[nodiscard]] qint64 pos() const;

    [[nodiscard]] sol::object read(const sol::variadic_args &args);

    [[nodiscard]] qint64 seek(const sol::optional<std::string> &whence, const sol::optional<qint64> &offset);

    [[nodiscard]] qint64 size();

    [[nodiscard]] File *write(const sol::variadic_args &args);

private:
    void ensureOpen() const;

    QFile m_file{};
    std::unique_ptr<QTextStream> m_textStream{};
    bool m_binary{};
};

#endif //UNICOMM_FILE_H
