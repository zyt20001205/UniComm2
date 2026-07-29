#include "api/file.h"

#include <QDataStream>
#include <sol/error.hpp>
#include <sol/variadic_args.hpp>

#include "util/uniCast.h"

// public
File::File(const QString &path, const std::string &mode) {
    QString _mode = QString::fromStdString(mode);
    if (_mode.endsWith('b')) {
        m_binary = true;
        _mode.chop(1);
    }

    QIODeviceBase::OpenMode flags{};
    if (_mode == "r") flags = QIODevice::ReadOnly;
    else if (_mode == "r+") flags = QIODevice::ReadWrite;
    else if (_mode == "w") flags = QIODevice::WriteOnly | QIODevice::Truncate;
    else if (_mode == "w+") flags = QIODevice::ReadWrite | QIODevice::Truncate;
    else if (_mode == "a") flags = QIODevice::WriteOnly | QIODevice::Append;
    else if (_mode == "a+") flags = QIODevice::ReadWrite | QIODevice::Append;
    else throw sol::error("invalid open mode: " + mode);
    if (!m_binary) flags |= QIODevice::Text;

    m_file.setFileName(path);
    if (!m_file.open(flags)) throw sol::error("file open failed: " + m_file.errorString().toStdString());
}

bool File::close() {
    if (!m_file.isOpen()) return true;
    m_file.close();
    return true;
}

bool File::flush() {
    if (!m_file.isOpen()) throw sol::error("file is closed");
    if (!m_file.isWritable()) return true;
    if (!m_file.flush()) throw sol::error("file flush failed: " + m_file.errorString().toStdString());
    return true;
}

sol::object File::read(const sol::variadic_args &args) {
    if (!m_file.isOpen()) throw sol::error("file is closed");

    QVariantList results{};
    auto formats = uni_cast<QVariantList>(args);
    if (formats.isEmpty()) formats = QVariantList({"l"});
    if (m_binary) {
        QDataStream stream(&m_file);
        for (const auto &format: formats) {
            if (format.typeId() == QMetaType::Int) {
                const int length = format.toInt();
                QByteArray data(length, Qt::Uninitialized);
                const auto read = stream.readRawData(data.data(), length);
                if (read <= 0) results.append(QVariant{});
                else results.append(data.left(static_cast<int>(read)));
            } else if (format.typeId() == QMetaType::QString && format.toString() == "a") {
                results.append(m_file.readAll());
            } else {
                throw sol::error("invalid read mode: " + format.toString().toStdString());
            }
        }
    } else {
        QTextStream stream(&m_file);
        for (const auto &format: formats) {
            if (format.typeId() == QMetaType::Int) {
                const auto data = stream.read(format.toInt());
                results.append(data.isNull() ? QVariant{} : QVariant{data});
            } else if (format.typeId() == QMetaType::QString) {
                const auto value = format.toString();
                if (value == "n") {
                    double number{};
                    stream >> number;
                    results.append(stream.status() == QTextStream::Ok ? QVariant{number} : QVariant{});
                } else if (value == "a") {
                    results.append(stream.readAll());
                } else if (value == "l") {
                    results.append(stream.atEnd() ? QVariant{} : QVariant{stream.readLine()});
                } else if (value == "L") {
                    results.append(stream.atEnd() ? QVariant{} : QVariant{stream.readLine() + "\n"});
                } else {
                    throw sol::error("invalid read format: " + value.toStdString());
                }
            } else {
                throw sol::error("invalid read mode: " + format.toString().toStdString());
            }
        }
    }

    return uni_cast<sol::object>(args.lua_state(), results);
}

qint64 File::seek(const sol::optional<std::string> &whence, const sol::optional<qint64> &offset) {
    if (!m_file.isOpen()) throw sol::error("file is closed");

    const auto origin = whence.value_or("cur");
    qint64 base{};
    if (origin == "set") base = 0;
    else if (origin == "cur") base = m_file.pos();
    else if (origin == "end") base = m_file.size();
    else throw sol::error("invalid seek origin: " + origin);

    const auto target = base + offset.value_or(0);
    if (!m_file.seek(target)) throw sol::error("file seek failed: " + m_file.errorString().toStdString());
    return target;
}

File *File::write(const sol::variadic_args &args) {
    if (!m_file.isOpen()) throw sol::error("file is closed");

    if (m_binary) {
        QDataStream stream(&m_file);
        for (const sol::object &arg: args) {
            const auto bytes = arg.as<std::string>();
            stream.writeRawData(bytes.data(), static_cast<int>(bytes.size()));
        }
        if (stream.status() != QDataStream::Ok)
            throw sol::error("file write failed: " + m_file.errorString().toStdString());
    } else {
        QTextStream stream(&m_file);
        for (const auto &arg: uni_cast<QVariantList>(args)) stream << arg.toString();
        stream.flush();
        if (m_file.error() != QFileDevice::NoError)
            throw sol::error("file write failed: " + m_file.errorString().toStdString());
    }

    return this;
}
