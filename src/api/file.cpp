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

sol::object File::read(const std::string &path, const sol::variadic_args &args) {
    const LPath luaPath = QString::fromStdString(path);
    const auto documentUrl = uni_cast<QUrl>(luaPath);
    if (!m_handleHash.contains(documentUrl)) throw sol::error("file not opened: " + path);

    const auto &handle = m_handleHash[documentUrl];
    QVariantList results{};
    auto _args = uni_cast<QVariantList>(args);
    if (_args.isEmpty()) _args = QVariantList({"l"});
    if (handle->binary) {
        QDataStream stream(&handle->file);
        for (const auto &arg: _args) {
            if (arg.typeId() == QMetaType::Int) {
                const int n = arg.toInt();
                QByteArray data(n, Qt::Uninitialized);
                const auto read = stream.readRawData(data.data(), n);
                if (read <= 0) results.append(QVariant());
                else results.append(QVariant(data.left(static_cast<int>(read))));
            } else if (arg.typeId() == QMetaType::QString && arg.toString() == "a") {
                results.append(QVariant(handle->file.readAll()));
            } else throw sol::error("invalid read mode: " + arg.toString().toStdString());
        }
    } else {
        QTextStream stream(&handle->file);
        for (const auto &arg: _args) {
            if (arg.typeId() == QMetaType::Int) {
                const auto data = stream.read(arg.toInt());
                results.append(data.isNull() ? QVariant() : QVariant(data));
            } else if (arg.typeId() == QMetaType::QString) {
                const auto _arg = arg.toString();
                if (_arg == "n") {
                    double n{};
                    stream >> n;
                    results.append(stream.status() == QTextStream::Ok ? QVariant(n) : QVariant());
                } else if (_arg == "a") {
                    results.append(QVariant(stream.readAll()));
                } else if (_arg == "l") {
                    results.append(stream.atEnd() ? QVariant() : QVariant(stream.readLine()));
                } else if (_arg == "L") {
                    results.append(stream.atEnd() ? QVariant() : QVariant(stream.readLine() + "\n"));
                } else throw sol::error("invalid read format: " + _arg.toStdString());
            } else throw sol::error("invalid read mode: " + arg.toString().toStdString());
        }
    }
    return uni_cast<sol::object>(args.lua_state(), results);
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
