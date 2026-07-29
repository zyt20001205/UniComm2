#include "api/file.h"

#include <QTextStream>
#include <sol/error.hpp>

#include <limits>

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
    if (!m_binary) m_textStream = std::make_unique<QTextStream>(&m_file);
}

File::~File() {
    if (!m_file.isOpen()) return;
    if (m_textStream) m_textStream->flush();
    m_textStream.reset();
    m_file.close();
}

bool File::atEnd() const {
    ensureOpen();
    if (m_textStream) return m_textStream->atEnd();
    return m_file.atEnd();
}

bool File::close() {
    if (!m_file.isOpen()) return true;

    try {
        if (m_file.isWritable()) flush();
    } catch (...) {
        m_textStream.reset();
        m_file.close();
        throw;
    }

    m_textStream.reset();
    m_file.close();
    return true;
}

bool File::flush() {
    ensureOpen();
    if (!m_file.isWritable()) return true;

    if (m_textStream) {
        m_textStream->flush();
        if (m_textStream->status() == QTextStream::WriteFailed)
            throw sol::error("file flush failed: " + m_file.errorString().toStdString());
        return true;
    }

    if (!m_file.flush()) throw sol::error("file flush failed: " + m_file.errorString().toStdString());
    return true;
}

qint64 File::pos() const {
    ensureOpen();
    if (m_textStream) return m_textStream->pos();
    return m_file.pos();
}

sol::object File::read(const sol::variadic_args &args) {
    ensureOpen();
    if (!m_file.isReadable()) throw sol::error("file is not readable");

    QVariantList results{};
    auto formats = uni_cast<QVariantList>(args);
    if (formats.isEmpty()) formats.append("l");

    if (m_binary) {
        for (const auto &format: formats) {
            if (format.typeId() == QMetaType::Int || format.typeId() == QMetaType::LongLong) {
                const auto length = format.toLongLong();
                if (length < 0 || length > QByteArray::maxSize()) throw sol::error("invalid read length");
                if (length == 0) {
                    results.append(m_file.atEnd() ? QVariant{} : QVariant{QByteArray{}});
                    continue;
                }

                const auto data = m_file.read(length);
                if (m_file.error() != QFileDevice::NoError)
                    throw sol::error("file read failed: " + m_file.errorString().toStdString());
                results.append(data.isEmpty() && m_file.atEnd() ? QVariant{} : QVariant{data});
                continue;
            }

            if (format.typeId() == QMetaType::QString && format.toString() == "a") {
                const auto data = m_file.readAll();
                if (m_file.error() != QFileDevice::NoError)
                    throw sol::error("file read failed: " + m_file.errorString().toStdString());
                results.append(data);
                continue;
            }

            throw sol::error("invalid binary read format: " + format.toString().toStdString());
        }
    } else {
        for (const auto &format: formats) {
            if (format.typeId() == QMetaType::Int || format.typeId() == QMetaType::LongLong) {
                const auto length = format.toLongLong();
                if (length < 0 || length > std::numeric_limits<qsizetype>::max()) throw sol::error("invalid read length");
                if (length == 0) {
                    results.append(m_textStream->atEnd() ? QVariant{} : QVariant{QString{}});
                    continue;
                }
                const auto data = m_textStream->read(length);
                results.append(data.isNull() ? QVariant{} : QVariant{data});
                continue;
            }

            if (format.typeId() != QMetaType::QString)
                throw sol::error("invalid read format: " + format.toString().toStdString());

            const auto value = format.toString();
            if (value == "n") {
                double number{};
                *m_textStream >> number;
                if (m_textStream->status() == QTextStream::Ok) {
                    results.append(number);
                } else {
                    results.append(QVariant{});
                    m_textStream->resetStatus();
                }
            } else if (value == "a") {
                results.append(m_textStream->readAll());
            } else if (value == "l") {
                results.append(m_textStream->atEnd() ? QVariant{} : QVariant{m_textStream->readLine()});
            } else if (value == "L") {
                results.append(m_textStream->atEnd() ? QVariant{} : QVariant{m_textStream->readLine() + '\n'});
            } else {
                throw sol::error("invalid read format: " + value.toStdString());
            }
        }
    }

    return uni_cast<sol::object>(args.lua_state(), results);
}

qint64 File::seek(const sol::optional<std::string> &whence, const sol::optional<qint64> &offset) {
    ensureOpen();
    if (m_file.isWritable()) flush();

    const auto origin = whence.value_or("cur");
    qint64 base{};
    if (origin == "set") base = 0;
    else if (origin == "cur") base = pos();
    else if (origin == "end") base = size();
    else throw sol::error("invalid seek origin: " + origin);

    const auto delta = offset.value_or(0);
    if (delta < 0) {
        if (delta < -base) throw sol::error("invalid seek offset");
    } else if (base > std::numeric_limits<qint64>::max() - delta) {
        throw sol::error("invalid seek offset");
    }
    const auto target = base + delta;

    const bool success = m_textStream ? m_textStream->seek(target) : m_file.seek(target);
    if (!success) throw sol::error("file seek failed: " + m_file.errorString().toStdString());
    return target;
}

qint64 File::size() {
    ensureOpen();
    if (m_file.isWritable()) flush();
    return m_file.size();
}

File *File::write(const sol::variadic_args &args) {
    ensureOpen();
    if (!m_file.isWritable()) throw sol::error("file is not writable");

    for (const sol::object &arg: args) {
        QByteArray bytes{};
        if (arg.is<std::string>()) {
            const auto value = arg.as<std::string>();
            bytes = QByteArray(value.data(), static_cast<qsizetype>(value.size()));
        } else if (arg.is<lua_Integer>()) {
            bytes = QByteArray::number(arg.as<lua_Integer>());
        } else if (arg.is<lua_Number>()) {
            bytes = QByteArray::number(arg.as<lua_Number>(), 'g', 14);
        } else {
            throw sol::error("file write expects string or number");
        }

        if (m_binary) {
            qsizetype written{};
            while (written < bytes.size()) {
                const auto length = m_file.write(bytes.constData() + written, bytes.size() - written);
                if (length <= 0) throw sol::error("file write failed: " + m_file.errorString().toStdString());
                written += length;
            }
        } else {
            *m_textStream << QString::fromUtf8(bytes);
            if (m_textStream->status() == QTextStream::WriteFailed)
                throw sol::error("file write failed: " + m_file.errorString().toStdString());
        }
    }

    return this;
}

// private
void File::ensureOpen() const {
    if (!m_file.isOpen()) throw sol::error("file is closed");
}
