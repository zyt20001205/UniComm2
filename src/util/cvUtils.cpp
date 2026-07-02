#include "util/cvUtils.h"

#include <QJsonArray>

#include <opencv2/core/mat.hpp>

#include "globals.h"

namespace {
    cv::Mat pixmapToGrayMat(const QPixmap &pixmap) {
        const QImage image = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
        const cv::Mat gray(image.height(), image.width(), CV_8UC1, const_cast<uchar *>(image.bits()), image.bytesPerLine());
        return gray.clone();
    }
}

QPixmap pipelineProcess(const QPixmap &pixmap, const QJsonArray &pipeline) {
    QImage image = pixmap.toImage();
    cv::Mat frame(image.height(), image.width(),
                  image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
                  image.bits(),
                  image.bytesPerLine());
    for (const auto &value: pipeline) {
        const auto &session = value.toObject();
        const int type = session["type"].toInt();
        switch (type) {
            case ImagePipeline::Scale: {
                const int ratio = session["ratio"].toInt();
                float f_ratio = 1;
                switch (ratio) {
                    case -5: {
                        f_ratio = 0.1;
                    }
                    break;
                    case -4: {
                        f_ratio = 0.25;
                    }
                    break;
                    case -3: {
                        f_ratio = 0.3;
                    }
                    break;
                    case -2: {
                        f_ratio = 0.5;
                    }
                    break;
                    case -1: {
                        f_ratio = 0.75;
                    }
                    break;
                    case 0: {
                        f_ratio = 1;
                    }
                    break;
                    case 1: {
                        f_ratio = 1.5;
                    }
                    break;
                    case 2: {
                        f_ratio = 2;
                    }
                    break;
                    case 3: {
                        f_ratio = 3;
                    }
                    break;
                    case 4: {
                        f_ratio = 5;
                    }
                    break;
                    case 5: {
                        f_ratio = 10;
                    }
                    break;
                    default: break;;
                }
                const double interpolation = session["interpolation"].toInt();
                frame = scale(frame, f_ratio, interpolation);
            }
            break;
            case ImagePipeline::Threshold: {
                const int thresh = session["thresh"].toInt();
                const int maxval = session["maxval"].toInt();
                const int mode = session["mode"].toInt();
                frame = threshold(frame, thresh, maxval, mode);
            }
            break;
            default: break;
        }
    }
    const QImage::Format format = frame.channels() == 1 ? QImage::Format_Grayscale8 : image.format();
    const QImage processedImage(frame.data, frame.cols, frame.rows, frame.step, format);
    return QPixmap::fromImage(processedImage.copy());
}

cv::Mat scale(const cv::Mat &input, const float ratio, const int interpolation) {
    const int newWidth = static_cast<int>(input.cols * ratio);
    const int newHeight = static_cast<int>(input.rows * ratio);
    cv::Mat output{};
    cv::resize(input, output, cv::Size(newWidth, newHeight), 0, 0, interpolation);
    return output;
}

cv::Mat threshold(const cv::Mat &input, const int thresh, const int maxval, const int mode) {
    cv::Mat output{};
    if (input.channels() != 1) {
        cv::Mat gray{};
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, output, thresh, maxval, mode);
    } else {
        cv::threshold(input, output, thresh, maxval, mode);
    }

    return output;
}

QPoint goodFeaturesToTrack(const QPixmap &pixmap) {
    const cv::Mat gray = pixmapToGrayMat(pixmap);
    std::vector<cv::Point2f> corners{};
    cv::goodFeaturesToTrack(gray, corners, 1, 0.01, 10);
    if (corners.empty()) {
        return QPoint(-1, -1);
    }

    return QPoint(qRound(corners.front().x), qRound(corners.front().y));
}

QPoint harris(const QPixmap &pixmap) {
    const cv::Mat gray = pixmapToGrayMat(pixmap);
    cv::Mat response{};
    cv::cornerHarris(gray, response, 2, 3, 0.04);

    double maxValue = 0;
    cv::Point maxLocation{};
    cv::minMaxLoc(response, nullptr, &maxValue, nullptr, &maxLocation);
    if (maxValue <= 0) {
        return QPoint(-1, -1);
    }

    return QPoint(maxLocation.x, maxLocation.y);
}
