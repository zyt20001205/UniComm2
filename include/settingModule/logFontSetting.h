#ifndef UNICOMM_LOGFONTSETTING_H
#define UNICOMM_LOGFONTSETTING_H

#include <QWidget>

class QSpinBox;
class QFontComboBox;

class LogFontSetting final : public QWidget {
    Q_OBJECT

public:
    explicit LogFontSetting(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &logFontConfig) const;

    ~LogFontSetting() override = default;

    QJsonObject settingExport() const;

private:
    QFontComboBox *m_fontFamilyComboBox{};
    QSpinBox *m_fontSizeSpinBox{};
};

#endif //UNICOMM_LOGFONTSETTING_H
