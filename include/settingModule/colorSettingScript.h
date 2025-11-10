#ifndef UNICOMM_COLORSETTINGSCRIPT_H
#define UNICOMM_COLORSETTINGSCRIPT_H

#include <QTextEdit>
#include <QWidget>

class QPushButton;

class ColorSettingScript final : public QWidget {
    Q_OBJECT

public:
    explicit ColorSettingScript(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &colorConfigScript) const;

    ~ColorSettingScript() override = default;

    QJsonObject settingExport() const;

private:
    QPushButton *m_backgroundErrorButton{};
    QPushButton *m_backgroundWarningButton{};
    QPushButton *m_backgroundInfoButton{};
    QPushButton *m_backgroundHintButton{};
    QTextEdit *m_colorPreviewTextEdit{};
};

#endif //UNICOMM_COLORSETTINGSCRIPT_H
