#include "luaModule/luaSmtp.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <sol/error.hpp>

#include "globals.h"
#include "portModule/basePort.h"
#include "portModule/portModule.h"
#include "utils/luaUtils.h"

LuaSmtp::LuaSmtp(QObject *parent)
    : QObject(parent) {
}

void LuaSmtp::ehlo(const std::string &portName) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "EHLO localhost";
    bool status = false;

    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP EHLO failed");
    }
}

void LuaSmtp::authLogin(const std::string &portName, const std::string &username, const std::string &password) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "AUTH LOGIN";
    bool status = false;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("334")) {
        throw sol::error(portName + ": SMTP AUTH LOGIN failed");
    }

    txData = QByteArray::fromStdString(username).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("334")) {
        throw sol::error(portName + ": SMTP invalid username");
    }

    txData = QByteArray::fromStdString(password).toBase64();
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("235")) {
        throw sol::error(portName + ": SMTP invalid password");
    }
}

void LuaSmtp::mail(const std::string &portName, const std::string &from, const std::string &to, const std::string &subject, const std::string &body,
                   const std::string &attachment) {
    if (!g_port->m_portHash.contains(QString::fromStdString(portName))) {
        throw sol::error(portName + " does not exist");
    }

    auto *port = g_port->m_portHash[QString::fromStdString(portName)];
    QByteArray txData = "MAIL FROM: <" + QByteArray::fromStdString(from) + ">";
    bool status = false;
    QByteArray rxData{};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP MAIL FROM failed");
    }

    txData = "RCPT TO: <" + QByteArray::fromStdString(to) + ">";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP RCPT failed");
    }

    txData = "DATA";
    status = false;
    rxData = {};

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("354")) {
        throw sol::error(portName + ": SMTP DATA failed");
    }

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

    QMetaObject::invokeMethod(port, [&port, &txData, &status, &rxData] {
        status = port->write(txData, "ascii", "crlf");
        port->clear();
        rxData = port->read(3, 1000, "ascii");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
    if (!QString::fromLatin1(rxData).startsWith("250")) {
        throw sol::error(portName + ": SMTP MAIL failed");
    }

    txData = "QUIT";
    status = false;

    QMetaObject::invokeMethod(port, [&port, &txData, &status] {
        status = port->write(txData, "ascii", "crlf");
    }, Qt::BlockingQueuedConnection);
    if (!status || rxData.isEmpty()) {
        throw sol::error(portName + ": communication failed");
    }
}
