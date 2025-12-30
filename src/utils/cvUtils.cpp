#include "utils/cvUtils.h"

#include <QJsonArray>
#include <opencv2/core/mat.hpp>

#include "globals.h"

QPixmap processPipeline(const QPixmap &pixmap, const QJsonArray &pipeline) {
    QImage image = pixmap.toImage();
    cv::Mat intermediate(image.height(), image.width(),
                  image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
                  image.bits(),
                  image.bytesPerLine());
    for (const auto &value: pipeline) {
        const auto &session = value.toObject();
        const int type = session["type"].toInt();
        switch (type) {
            case SCALE: {
                const double ratio = session["ratio"].toDouble();
                intermediate = scale(intermediate, ratio);
            }
            break;
            default: break;
        }
    }
    const QImage processedImage(intermediate.data,intermediate.cols,intermediate.rows,intermediate.step,image.format());
    return QPixmap::fromImage(processedImage.copy());
}

cv::Mat scale(const cv::Mat &input, const double ratio, const int interpolation) {
    const int newWidth = static_cast<int>(input.cols * ratio);
    const int newHeight = static_cast<int>(input.rows * ratio);
    cv::Mat output;
    cv::resize(input, output, cv::Size(newWidth, newHeight), 0, 0, interpolation);
    return output;
}

// QPixmap processThreshold(const QPixmap &pixmap, const int thresh, const int type) {
//     QImage image = pixmap.toImage();
//     const cv::Mat cvImg(image.height(), image.width(),
//                         image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
//                         image.bits(),
//                         image.bytesPerLine());
//     cv::Mat processed;
//     cv::Mat gray;
//     cv::cvtColor(cvImg, gray, cv::COLOR_BGRA2GRAY);
//     cv::threshold(gray, processed, thresh, 255, type);
//     cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGRA);
//     const QImage result(
//         processed.data,
//         processed.cols,
//         processed.rows,
//         processed.step,
//         image.format()
//     );
//     return QPixmap::fromImage(result.copy());
// }
