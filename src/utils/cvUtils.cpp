#include "utils/cvUtils.h"

#include <QUrl>
#include "baseapi.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/mat.hpp"

QString ocr(const QPixmap &pixmap, const QString &charset) {
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    const char *tessCharset = charset.isEmpty() ? "eng" : charset.toUtf8().constData();
    auto *ocr = new tesseract::TessBaseAPI();
    ocr->Init(nullptr, tessCharset);
    ocr->SetImage(image.bits(), image.width(), image.height(), 3, image.bytesPerLine());
    char *result = ocr->GetUTF8Text();
    QString text = QString::fromUtf8(result);
    delete result;
    ocr->End();
    delete ocr;
    text = text.trimmed();
    return text.isEmpty() ? "null" : text;
}

QPixmap processGaussianBlur(const QPixmap &pixmap, const int size) {
    QImage image = pixmap.toImage();
    const cv::Mat cvImg(image.height(), image.width(),
                        image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
                        image.bits(),
                        image.bytesPerLine());
    cv::Mat processed;
    cv::GaussianBlur(cvImg, processed, cv::Size(2 * size + 1, 2 * size + 1), 0);
    const QImage result(
        processed.data,
        processed.cols,
        processed.rows,
        processed.step,
        image.format()
    );
    return QPixmap::fromImage(result.copy());
}

QPixmap processThreshold(const QPixmap &pixmap, const int thresh, const int type) {
    QImage image = pixmap.toImage();
    const cv::Mat cvImg(image.height(), image.width(),
                        image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
                        image.bits(),
                        image.bytesPerLine());
    cv::Mat processed;
    cv::Mat gray;
    cv::cvtColor(cvImg, gray, cv::COLOR_BGRA2GRAY);
    cv::threshold(gray, processed, thresh, 255, type);
    cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGRA);
    const QImage result(
        processed.data,
        processed.cols,
        processed.rows,
        processed.step,
        image.format()
    );
    return QPixmap::fromImage(result.copy());
}