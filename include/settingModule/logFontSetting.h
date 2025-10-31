#ifndef UNICOMM_LOGFONTSETTING_H
#define UNICOMM_LOGFONTSETTING_H

#include <QTextEdit>
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
    QTextEdit *m_fontPreviewTextEdit{};
};

#endif //UNICOMM_LOGFONTSETTING_H
