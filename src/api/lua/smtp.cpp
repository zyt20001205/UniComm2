#include "api/lua/smtp.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <sol/error.hpp>

#include "globals.h"
#include "port/basePort.h"
#include "port/portModule.h"
#include "utils/luaUtils.h"

// public
Smtp::Smtp(QObject *parent)
    : QObject(parent) {
}

void Smtp::ehlo(const std::string &portName, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&exception, &port, &timeout] {
        QByteArray rxData{};

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write("EHLO localhost", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void Smtp::authLogin(const std::string &portName, const std::string &username, const std::string &password, const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData1 = QByteArray::fromStdString(username).toBase64();
    const auto txData2 = QByteArray::fromStdString(password).toBase64();

    QMetaObject::invokeMethod(port, [&exception, &port, &txData1, &txData2, &timeout] {
        QByteArray rxData{};

        if (!port->write("AUTH LOGIN", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write(txData1, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write(txData2, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void Smtp::send(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachment,
                const int timeout) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    const auto txData1 = "MAIL FROM: <" + QByteArray::fromStdString(from) + ">";
    const auto txData2 = "RCPT TO: <" + QByteArray::fromStdString(to) + ">";
    const auto boundary = QString::number(QDateTime::currentMSecsSinceEpoch());
    QByteArray txData3 =
            "From: " + QByteArray::fromStdString(from) + "\r\n"
            + "To: " + QByteArray::fromStdString(to) + "\r\n"
            + "Subject: " + QByteArray::fromStdString(subject) + "\r\n"
            + "Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8() + "\r\n"
            + "MIME-Version: 1.0\r\n"
            + "Content-Type: multipart/mixed; boundary=\"" + boundary.toUtf8() + "\"\r\n"
            + "\r\n"
            + "--" + boundary.toUtf8() + "\r\n"
            + "Content-Type: text/plain; charset=utf-8\r\n"
            + "Content-Transfer-Encoding: 8bit\r\n"
            + "\r\n"
            + QByteArray::fromStdString(body) + "\r\n";
    if (!attachment.empty()) {
        const auto &filePath = lua2filepath(attachment);
        const QMimeDatabase mimeDb{};
        const auto &mimeType = mimeDb.mimeTypeForFile(filePath).name();
        const auto &fileName = QFileInfo(filePath).fileName();

        txData3 +=
                "--" + boundary.toUtf8() + "\r\n"
                + "Content-Type: " + mimeType.toUtf8() + "\r\n"
                + "Content-Transfer-Encoding: base64\r\n"
                + "Content-Disposition: attachment; filename=\"" + fileName.toUtf8() + "\"\r\n"
                + "\r\n";

        QFile attachmentFile(filePath);
        if (attachmentFile.open(QIODevice::ReadOnly)) {
            const auto base64Data = attachmentFile.readAll().toBase64();
            txData3 += base64Data + "\r\n";
        } else {
            throw sol::error(portName + ": attachment invalid");
        }
    }
    txData3 += "--" + boundary.toUtf8() + "--\r\n.";


    QMetaObject::invokeMethod(port, [&exception, &port, &txData1, &txData2, &txData3, &timeout] {
        QByteArray rxData{};

        if (!port->write(txData1, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write(txData2, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write("DATA", "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
        else return;

        if (!port->write(txData3, "utf-8", "crlf")) {
            exception = "write failed";
            return;
        }

        while (exception.isEmpty()) {
            rxData = port->readUntil("\r\n", timeout, "utf-8");
            exception = parser(rxData);
        }
        if (exception == "end") exception = "";
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

void Smtp::quit(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) throw sol::error(portName + " does not exist");

    QString exception{};
    auto *port = g_port->m_portHash[QString::fromStdString(portName)];

    QMetaObject::invokeMethod(port, [&exception, &port] {
        if (!port->write("QUIT", "utf-8", "crlf")) {
            exception = "write failed";
        }
    }, Qt::BlockingQueuedConnection);
    if (!exception.isEmpty()) throw sol::error(portName + ": " + exception.toStdString());
}

// private
QString Smtp::parser(const QByteArray &rxData) {
    if (rxData.isEmpty()) return "read timeout";
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
        default: return "contact author: unsupported status code(" + QString::number(code) + ")";
    }
    if (rxData.at(3) != '-') {
        return "end";
    }
    return {};
}
