#ifndef UNICOMM_FONTSETTINGSCRIPT_H
#define UNICOMM_FONTSETTINGSCRIPT_H

#include <QTextEdit>
#include <QWidget>

class QSpinBox;
class QFontComboBox;

class FontSettingScript final : public QWidget {
    Q_OBJECT

public:
    explicit FontSettingScript(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &fontConfigScript) const;

    ~FontSettingScript() override = default;

    QJsonObject settingExport() const;

private:
    QFontComboBox *m_fontFamilyComboBox{};
    QSpinBox *m_fontSizeSpinBox{};
    QTextEdit *m_fontPreviewTextEdit{};
};

#endif //UNICOMM_FONTSETTINGSCRIPT_H
