#ifndef UNICOMM_FONTSETTINGLOG_H
#define UNICOMM_FONTSETTINGLOG_H

#include <QTextEdit>
#include <QWidget>

class QSpinBox;
class QFontComboBox;

class FontSettingLog final : public QWidget {
    Q_OBJECT

public:
    explicit FontSettingLog(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &fontConfigLog) const;

    ~FontSettingLog() override = default;

    QJsonObject settingExport() const;

private:
    QFontComboBox *m_fontFamilyComboBox{};
    QSpinBox *m_fontSizeSpinBox{};
    QTextEdit *m_fontPreviewTextEdit{};
};

#endif //UNICOMM_FONTSETTINGLOG_H
