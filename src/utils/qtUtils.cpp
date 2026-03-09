#include "utils/qtUtils.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTextDocument>
#include <QUrl>

QByteArray fileHashCalc(const QString &fileInfo) {
    QFile file(fileInfo);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    if (QCryptographicHash hash(QCryptographicHash::Sha256); hash.addData(&file)) {
        return hash.result();
    }
    return QByteArray();
}

QByteArray fileHashCalc(const QUrl &fileInfo) {
    const QString filePath = fileInfo.toLocalFile();
    return fileHashCalc(filePath);
}

QString md2plain(const QString &markdown) {
    QTextDocument doc{};
    doc.setMarkdown(markdown.toHtmlEscaped());
    const QString plain = doc.toPlainText();
    return plain;
}