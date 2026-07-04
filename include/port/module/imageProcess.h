#ifndef UNICOMM_IMAGEPROCESS_H
#define UNICOMM_IMAGEPROCESS_H

#include <QImage>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QUrl>

#include "opencv2/core/mat.hpp"

namespace tesseract {
    class TessBaseAPI;
}

class ImageProcess final : public QObject {
    Q_OBJECT

public:
    struct ProcessResult {
        QImage roiFrame;
        QImage pipelineFrame;
        QString result;
    };

    explicit ImageProcess(QObject *parent = nullptr);

    ~ImageProcess() override;

    void configSetGlobal(const QJsonObject &config) {
        m_config = config;
    }

    void configSetRoi(const QJsonArray &roi) {
        m_config["roi"] = roi;
    }

    void configSetPipeline(const QJsonArray &pipeline) {
        m_config["pipeline"] = pipeline;
    }

    void configSetRecognition(const QJsonObject &recognition) {
        m_config["recognition"] = recognition;
    }

    [[nodiscard]] QStringList process(const QImage &frame);

    [[nodiscard]] QList<ProcessResult> detail(const QImage &frame);

private:
    [[nodiscard]] static QImage roi(const QImage &frame, const QJsonArray &roi);

    [[nodiscard]] static QImage pipeline(const QImage &roiFrame, const QJsonArray &pipeline);

    [[nodiscard]] QString recognition(const QImage &pipelineFrame, const QJsonObject &recognition);

    [[nodiscard]] static cv::Mat scale(const cv::Mat &input, float ratio, int interpolation);

    [[nodiscard]] static cv::Mat threshold(const cv::Mat &input, int thresh, int maxval, int mode);

    [[nodiscard]] static QPoint goodFeaturesToTrack(const QImage &image);

    [[nodiscard]] static QPoint cornerHarris(const QImage &image);

    [[nodiscard]] static QPoint templateMatch(const QImage &image, const QImage &t_image);

    QJsonObject m_config{};
    QUrl m_templateUrl{};
    QImage m_template{};
    tesseract::TessBaseAPI *m_ocrEngine{};
};

#endif //UNICOMM_IMAGEPROCESS_H
