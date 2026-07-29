#include "api/filesystem.h"

#include <QDir>
#include <QFile>
#include <sol/error.hpp>

#include "api/file.h"
#include "util/uniCast.h"

// public
std::shared_ptr<File> Filesystem::open(const std::string &path, const std::string &mode) {
    const auto documentUrl = uni_cast<QUrl>(LPath(QString::fromStdString(path)));
    return std::make_shared<File>(documentUrl.toLocalFile(), mode);
}

void Filesystem::remove(const std::string &path) {
    const auto documentUrl = uni_cast<QUrl>(LPath(QString::fromStdString(path)));
    if (!QFile::remove(documentUrl.toLocalFile())) throw sol::error("file remove failed: " + path);
}

void Filesystem::rename(const std::string &from, const std::string &to) {
    const auto sourceUrl = uni_cast<QUrl>(LPath(QString::fromStdString(from)));
    const auto targetUrl = uni_cast<QUrl>(LPath(QString::fromStdString(to)));
    if (!QDir().rename(sourceUrl.toLocalFile(), targetUrl.toLocalFile())) throw sol::error("path rename failed: " + from);
}
