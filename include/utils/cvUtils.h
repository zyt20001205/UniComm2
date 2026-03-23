#ifndef UNICOMM_CVUTILS_H
#define UNICOMM_CVUTILS_H

#include <QPixmap>

#include <opencv2/imgproc.hpp>

namespace cv {
    class Mat;
}

class QJsonArray;

QPixmap pipelineProcess(const QPixmap &pixmap, const QJsonArray &pipeline);

cv::Mat scale(const cv::Mat &input, float ratio, int interpolation);

cv::Mat threshold(const cv::Mat &input, int thresh, int mode);

#endif //UNICOMM_CVUTILS_H
