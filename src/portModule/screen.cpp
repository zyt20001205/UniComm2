#include "portModule/screen.h"

#include <QLabel>
#include <QScreen>

#include "globals.h"
#include "utils/cvUtils.h"

// Screen public
Screen::Screen(const QJsonObject &portConfig, QObject *parent)
    : BasePort(parent),
      m_portConfig(portConfig) {
}

int Screen::type() {
    return SCREEN;
}

QJsonObject Screen::config() {
    return m_portConfig;
}

bool Screen::open() {
    emit refreshPort(m_portConfig["portName"].toString(), true);
    emit appendLog(QString("%1 opened").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 opened").arg(timestamp, m_portConfig["portName"].toString());
    return true;
}

void Screen::close() {
    emit refreshPort(m_portConfig["portName"].toString(), false);
    emit appendLog(QString("%1 closed").arg(m_portConfig["portName"].toString()), "info");
    // logging
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_portConfig["portName"].toString());
}

std::unordered_map<std::string, std::string> Screen::info() {
    return {};
}

QByteArray Screen::read(const int timeout, const int length, const QString &rxFormat) {
    // find screen
    for (QScreen *s: QGuiApplication::screens()) {
        if (s->name() == m_portConfig["portName"].toString()) {
            m_screen = s;
            break;
        }
    }
    if (!m_screen) return "screen not found";
    const auto shot = m_screen->grabWindow(0);
    QStringList resultList{};
    for (const QJsonValue &value: m_portConfig["roi"].toArray()) {
        QJsonArray roi = value.toArray();
        const int x = roi[0].toInt();
        const int y = roi[1].toInt();
        const int width = roi[2].toInt();
        const int height = roi[3].toInt();
        const auto rect = QRect(x, y, width, height);
        const QPixmap cropped = shot.copy(rect);
        const QString text = ocr(cropped, "eng", m_portConfig["whitelist"].toString());
        resultList.append(text);
    }
    return resultList.join("\x1E").toUtf8();
}
