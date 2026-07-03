#ifndef UNICOMM_CVUTILS_H
#define UNICOMM_CVUTILS_H

#include <QPixmap>

#include <opencv2/imgproc.hpp>

class QJsonArray;

QPixmap pipelineProcess(const QPixmap &pixmap, const QJsonArray &pipeline);

cv::Mat scale(const cv::Mat &input, float ratio, int interpolation);

cv::Mat threshold(const cv::Mat &input, int thresh, int maxval, int mode);

QPoint goodFeaturesToTrack(const QPixmap &pixmap);

QPoint cornerHarris(const QPixmap &pixmap);

QPoint templateMatch(const QPixmap &pixmap, const QPixmap &t_pixmap);

#endif //UNICOMM_CVUTILS_H
