#include "portModule/screen.h"

#include <QScreen>

#include "globals.h"
#include "utils/cvUtils.h"

// Screen public
Screen::Screen(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portName(portConfig["portName"].toString()),
      m_charset(portConfig["charset"].toString()),
      m_process(portConfig["process"].toObject()),
      m_areaList(portConfig["areaList"].toArray()) {
}

int Screen::type() {
    return SCREEN;
}

QJsonObject Screen::config() {
    return {};
}

void Screen::reload(const QJsonObject &portConfig) {
    m_portName = portConfig["portName"].toString();
    m_charset = portConfig["charset"].toString();
    m_process = portConfig["process"].toObject();
    m_areaList = portConfig["areaList"].toArray();
}

bool Screen::open() {
    m_showPreview = true;
    emit togglePort(true);
    emit appendLog(QString("%1 opened").arg(m_portName), "info");
    emit showPreview({});
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portName);
    return true;
}

void Screen::close() {
    m_showPreview = false;
    emit togglePort(false);
    emit appendLog(QString("%1 closed").arg(m_portName), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portName);
}

std::unordered_map<std::string, std::string> Screen::info() {
    return {};
}

QByteArray Screen::read(const int timeout, const int length, const QString &rxFormat) {
    // find screen
    for (QScreen *s: QGuiApplication::screens()) {
        if (s->name() == m_portName) {
            m_screen = s;
            break;
        }
    }
    if (!m_screen) return "screen not found";
    const auto shot = m_screen->grabWindow(0);
    QList<QPixmap> pixmapList{};
    QStringList resultList;
    for (const QJsonValue &value: m_areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        QPixmap processed{};
        const int processType = m_process["processType"].toInt();
        if (processType == RAW) {
            processed = cropped;
        } else {
            switch (processType) {
                case GAUSSIANBLUR: {
                    const int kernalSize = m_process["thresholdValue"].toInt();
                    processed = processGaussianBlur(cropped, kernalSize);
                    break;
                }
                case THRESHOLD: {
                    const int thresholdValue = m_process["thresholdValue"].toInt();
                    const int thresholdType = m_process["thresholdType"].toInt();
                    processed = processThreshold(cropped, thresholdValue, thresholdType);
                    break;
                }
                default: break;
            }
        }
        if (m_showPreview) pixmapList.append(processed);
        const QString text = ocr(processed, m_charset);
        resultList.append(text);
    }
    emit showPreview(pixmapList);
    // return resultList.join("\x1E");
}