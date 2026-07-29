#include "api/filesystem.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <sol/error.hpp>

#include "api/file.h"
#include "globals.h"
#include "util/uniCast.h"

// public
std::shared_ptr<File> Filesystem::open(const std::string &path, const std::string &mode) {
    return std::make_shared<File>(resolve(path), mode);
}

bool Filesystem::exists(const std::string &path) {
    const QFileInfo info(resolve(path));
    return info.exists() || info.isSymbolicLink();
}

sol::table Filesystem::list(const sol::this_state ts, const sol::optional<std::string> &path) {
    const QDir directory(resolve(path.value_or(".")));
    if (!directory.exists()) throw sol::error("directory does not exist");

    QVariantList entries{};
    const auto infoList = directory.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDir::Name);
    entries.reserve(infoList.size());
    for (const auto &info: infoList) entries.append(fileInfo(info));
    return uni_cast<sol::table>(ts, entries);
}

sol::object Filesystem::stat(const sol::this_state ts, const std::string &path) {
    const QFileInfo info(resolve(path));
    if (!info.exists() && !info.isSymbolicLink()) throw sol::error("path does not exist: " + path);
    return uni_cast<sol::object>(ts, fileInfo(info));
}

void Filesystem::copy(const std::string &from, const std::string &to) {
    const auto source = resolve(from);
    const auto target = resolve(to);
    ensureMutable(source);
    ensureMutable(target);

    const QFileInfo info(source);
    if (!info.exists() || !info.isFile()) throw sol::error("source is not a file: " + from);
    if (QFileInfo::exists(target)) throw sol::error("target already exists: " + to);
    if (!QFile::copy(source, target)) throw sol::error("file copy failed: " + from);
}

void Filesystem::mkdir(const std::string &path) {
    const auto target = resolve(path);
    ensureMutable(target);
    if (!QDir().mkpath(target)) throw sol::error("directory create failed: " + path);
}

void Filesystem::remove(const std::string &path) {
    const auto target = resolve(path);
    ensureMutable(target);

    const QFileInfo info(target);
    if (!info.exists() && !info.isSymbolicLink()) throw sol::error("path does not exist: " + path);
    if (info.isDir() && !info.isSymbolicLink()) throw sol::error("path is a directory: " + path);
    if (!QFile::remove(target)) throw sol::error("file remove failed: " + path);
}

void Filesystem::rename(const std::string &from, const std::string &to) {
    const auto source = resolve(from);
    const auto target = resolve(to);
    ensureMutable(source);
    ensureMutable(target);

    const QFileInfo info(source);
    if (!info.exists() && !info.isSymbolicLink()) throw sol::error("path does not exist: " + from);
    if (QFileInfo::exists(target)) throw sol::error("target already exists: " + to);
    if (!QDir().rename(source, target)) throw sol::error("path rename failed: " + from);
}

void Filesystem::rmdir(const std::string &path) {
    const auto target = resolve(path);
    ensureMutable(target);

    const QFileInfo info(target);
    if (!info.exists() || !info.isDir() || info.isSymbolicLink())
        throw sol::error("path is not a directory: " + path);
    if (!QDir().rmdir(target)) throw sol::error("directory remove failed: " + path);
}

void Filesystem::openExternal(const std::string &path) {
    const auto target = resolve(path);
    const QFileInfo info(target);
    if (!info.exists() && !info.isSymbolicLink()) throw sol::error("path does not exist: " + path);
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(target)))
        throw sol::error("external open failed: " + path);
}

// private
QVariantHash Filesystem::fileInfo(const QFileInfo &info) {
    QString type{"unknown"};
    if (info.isSymbolicLink()) type = "link";
    else if (info.isFile()) type = "file";
    else if (info.isDir()) type = "directory";

    QVariantHash result{
        {"name", info.fileName()},
        {"type", type}
    };
    if (info.isFile()) result["size"] = info.size();
    if (info.lastModified().isValid())
        result["modified"] = info.lastModified().toUTC().toString("yyyyMMddHHmmss.zzz");
    return result;
}

QString Filesystem::resolve(const std::string &path) {
    auto relative = QDir::fromNativeSeparators(QString::fromStdString(path));
    if (relative.isEmpty() || relative.contains(QChar::Null) || QDir::isAbsolutePath(relative))
        throw sol::error("invalid workspace path: " + path);

    const auto workspace = QDir(g_workspaceUrl.toLocalFile()).absolutePath();
    const auto target = QDir::cleanPath(QDir(workspace).absoluteFilePath(relative));
#ifdef Q_OS_WIN
    constexpr auto sensitivity = Qt::CaseInsensitive;
#else
    constexpr auto sensitivity = Qt::CaseSensitive;
#endif
    if (target != workspace && !target.startsWith(workspace + '/', sensitivity))
        throw sol::error("path escapes workspace: " + path);
    return target;
}

void Filesystem::ensureMutable(const QString &path) {
    const auto workspace = QDir(g_workspaceUrl.toLocalFile()).absolutePath();
#ifdef Q_OS_WIN
    constexpr auto sensitivity = Qt::CaseInsensitive;
#else
    constexpr auto sensitivity = Qt::CaseSensitive;
#endif
    if (path.compare(workspace, sensitivity) == 0) throw sol::error("workspace root cannot be modified");
}
