#ifndef UNICOMM_UTILS_H
#define UNICOMM_UTILS_H

#include <QPixmap>

#include <opencv2/imgproc.hpp>

namespace cv {
    class Mat;
}

class QJsonArray;

QPixmap processPipeline(const QPixmap &pixmap, const QJsonArray &pipeline);

cv::Mat scale(const cv::Mat &input, double ratio, int interpolation = cv::InterpolationFlags::INTER_LINEAR);

// QPixmap processThreshold(const QPixmap &pixmap, int thresh, int type);

#endif //UNICOMM_UTILS_H
