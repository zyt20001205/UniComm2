#include "luaModule/smtp.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/luaUtils.h"

// public
Smtp::Smtp(QObject *parent)
    : QObject(parent) {
}

void Smtp::ehlo(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray rxData{};
    std::string exception{};

    QMetaObject::invokeMethod(port, [&port, &rxData, &exception] {
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    QByteArray txData = "EHLO localhost";
    bool status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);
}

void Smtp::authLogin(const std::string &portName, const std::string &username, const std::string &password) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "AUTH LOGIN";
    bool status = false;
    QByteArray rxData{};
    std::string exception{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    txData = QByteArray::fromStdString(username).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    txData = QByteArray::fromStdString(password).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);
}

void Smtp::mail(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachment) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "MAIL FROM: <" + QByteArray::fromStdString(from) + ">";
    bool status = false;
    QByteArray rxData{};
    std::string exception{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    txData = "RCPT TO: <" + QByteArray::fromStdString(to) + ">";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    txData = "DATA";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    QString boundary = QString::number(QDateTime::currentMSecsSinceEpoch());
    txData = "From: " + QByteArray::fromStdString(from) + "\r\n";
    txData += "To: " + QByteArray::fromStdString(to) + "\r\n";
    txData += "Subject: " + QByteArray::fromStdString(subject) + "\r\n";
    txData += "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n";
    txData += "MIME-Version: 1.0\r\n";
    txData += "Content-Type: multipart/mixed; boundary=\"" + boundary.toUtf8() + "\"\r\n";
    txData += "\r\n";
    txData += "--" + boundary.toUtf8() + "\r\n";
    txData += "Content-Type: text/plain; charset=utf-8\r\n";
    txData += "Content-Transfer-Encoding: 8bit\r\n";
    txData += "\r\n";
    txData += QByteArray::fromStdString(body) + "\r\n";
    if (!attachment.empty()) {
        const auto &filePath = lua2filepath(attachment);
        QMimeDatabase mimeDb;
        const auto &mimeType = mimeDb.mimeTypeForFile(filePath).name();
        const auto &fileName = QFileInfo(filePath).fileName();

        txData += "--" + boundary.toUtf8() + "\r\n";
        txData += "Content-Type: " + mimeType.toUtf8() + "\r\n";
        txData += "Content-Transfer-Encoding: base64\r\n";
        txData += "Content-Disposition: attachment; filename=\"" + fileName.toUtf8() + "\"\r\n";
        txData += "\r\n";

        QFile imageFile(filePath);
        if (imageFile.open(QIODevice::ReadOnly)) {
            QByteArray imageData = imageFile.readAll();
            QByteArray base64Data = imageData.toBase64();
            txData += base64Data + "\r\n";
        } else {
            throw sol::error(portName + ": attachment invalid");
        }
    }
    txData += "--" + boundary.toUtf8() + "--\r\n.";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData, &exception] {
        status = port->write(txData, "utf-8", "crlf");
        while (true) {
            rxData = port->readUntil("\r\n", 1000, "utf-8");
            exception = parse(rxData);
            if (!exception.empty()) {
                if (exception == "end") exception = "";
                break;
            }
        }
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
    if (!exception.empty()) throw sol::error(portName + ": " + exception);

    txData = "QUIT";
    status = false;

    QMetaObject::invokeMethod(port, [&port, &txData, &status] {
        status = port->write(txData, "utf-8", "crlf");
    }, Qt::BlockingQueuedConnection);
    if (!status) throw sol::error(portName + ": communication failed");
}

// private
std::string Smtp::parse(const QByteArray &rxData) {
    if (rxData.size() < 4) return "invalid smtp response";
    const auto code = rxData.left(3).toInt();
    switch (code) {
        case 220:
        case 235:
        case 250:
        case 334:
        case 354:
            break;
        case 535:
            return "authentication credentials invalid";
        default: return "contact author: unsupported status code(" + std::to_string(code) + ")";
    }
    if (rxData.at(3) != '-') {
        return "end";
    }
    return {};
}
