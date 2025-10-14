#include "utils/qtUtils.h"

#include <QCryptographicHash>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QSvgRenderer>
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

QByteArray stringHashCalc(const QString &content) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(content.toUtf8());
    return hash.result();
}

QIcon SvgIcon(const QString &svgPath, const QColor &color, const QSize &size) {
    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}