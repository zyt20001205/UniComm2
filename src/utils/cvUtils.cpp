#include "utils/cvUtils.h"

#include <QJsonArray>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect.hpp>

#include "globals.h"

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
            case SCALE: {
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
            case THRESHOLD: {
                const int thresh = session["thresh"].toInt();
                const int mode = session["mode"].toInt();
                frame = threshold(frame, thresh, mode);
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

cv::Mat threshold(const cv::Mat &input, const int thresh, const int mode) {
    cv::Mat output{};
    if (input.channels() != 1) {
        cv::Mat gray{};
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, output, thresh, 255, mode);
    } else {
        cv::threshold(input, output, thresh, 255, mode);
    }

    return output;
}

int headDetect(const QPixmap &pixmap, double confidence) {
    QImage image = pixmap.toImage();
    const cv::Mat frame(image.height(), image.width(),
                         image.format() == QImage::Format_RGB32 ? CV_8UC4 : CV_8UC3,
                         image.bits(),
                         image.bytesPerLine());
    cv::Mat gray{};
    cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
    static cv::HOGDescriptor hog{};
    static bool hogInitialized = false;
    if (!hogInitialized) {
        hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
        hogInitialized = true;
        qDebug() << "HOG detector initialized";
    }
    std::vector<cv::Rect> found{};
    std::vector<double> weights{};
    hog.detectMultiScale(gray, found, weights, 0,
                            cv::Size(8, 8), cv::Size(32, 32),
                            1.05, 2.0, false);
    int count = 0;
    for (size_t i = 0; i < found.size(); ++i) {
        if (weights.empty() || weights[i] >= confidence) {
            count++;
        }
    }
    qDebug() << "Total detected:" << count << "people";
    return count;
}