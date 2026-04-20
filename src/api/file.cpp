#include "api/file.h"

#include <QDataStream>
#include <QUrl>

#include "util/uniCast.h"

File::File(QObject *parent)
    : QObject(parent) {
}

void File::open(const std::string &path, const std::string &mode) {
    const LPath luaPath = QString::fromStdString(path);
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    if (m_handleHash.contains(documentUrl)) throw sol::error("file already opened: " + path);

    bool binary = false;
    QString _mode = QString::fromStdString(mode);
    if (_mode.endsWith('b')) {
        binary = true;
        _mode.chop(1);
    }
    QIODeviceBase::OpenMode flags;
    if (_mode == "r") flags = QIODevice::ReadOnly;
    else if (_mode == "r+") flags = QIODevice::ReadWrite;
    else if (_mode == "w") flags = QIODevice::WriteOnly | QIODevice::Truncate;
    else if (_mode == "w+") flags = QIODevice::ReadWrite | QIODevice::Truncate;
    else if (_mode == "a") flags = QIODevice::WriteOnly | QIODevice::Append;
    else if (_mode == "a+") flags = QIODevice::ReadWrite | QIODevice::Append;
    else throw sol::error("invalid open mode: " + mode);
    if (!binary) flags |= QIODevice::Text;

    const auto handle = std::make_shared<FileHandle>();
    handle->file.setFileName(documentUrl.toLocalFile());
    handle->binary = binary;
    if (!handle->file.open(flags)) throw sol::error("file open failed: " + handle->file.errorString().toStdString());

    m_handleHash.insert(documentUrl, handle);
}

void File::close(const std::string &path) {
    const LPath luaPath = QString::fromStdString(path);
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    if (!m_handleHash.contains(documentUrl)) throw sol::error("file not opened: " + path);

    m_handleHash[documentUrl]->file.close();
    m_handleHash.remove(documentUrl);
}

std::string File::read(const std::string &path) {
    return {};
}

void File::write(const std::string &path, const sol::variadic_args &args) {
    const LPath luaPath = QString::fromStdString(path);
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    if (!m_handleHash.contains(documentUrl)) throw sol::error("file not opened: " + path);

    const auto &handle = m_handleHash[documentUrl];
    if (handle->binary) {
        QDataStream stream(&handle->file);
        for (const auto &arg: args) {
            const auto bytes = arg.as<std::string>();
            stream.writeRawData(bytes.data(), static_cast<int>(bytes.size()));
        }
        if (stream.status() != QDataStream::Ok) throw sol::error("file write failed: " + handle->file.errorString().toStdString());
    } else {
        QTextStream stream(&handle->file);
        for (const auto &arg: uni_cast<QVariantList>(args)) {
            stream << arg.toString();
        }
        stream.flush();
        if (handle->file.error() != QFileDevice::NoError) throw sol::error("file write failed: " + handle->file.errorString().toStdString());
    }
}
