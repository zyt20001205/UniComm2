#ifndef UNICOMM_SETTINGMODULE_H
#define UNICOMM_SETTINGMODULE_H

#include <QDialog>
#include <QJsonObject>

class QStackedWidget;
class QTreeView;

class FontSettingLog;
class FontSettingScript;

class SettingModule final : public QDialog {
    Q_OBJECT

public:
    explicit SettingModule(QWidget *parent = nullptr);

    ~SettingModule() override = default;

    void settingImport(const QJsonObject &settingConfig) const;

signals:
    void reloadLogFont(const QJsonObject &fontConfigLog);

    void saveLogFont(const QJsonObject &fontConfigLog);

    void reloadScriptFont(const QJsonObject &fontConfigScript);

    void saveScriptFont(const QJsonObject &fontConfigScript);

private:
    void settingApply();

    void settingSave();

    QStackedWidget *m_settingStackedWidget{};
    FontSettingLog *m_fontSettingLogWidget{};
    FontSettingScript *m_fontSettingScriptWidget{};

    enum {
        BLANK_SETTING,
        FONT_SETTING_LOG,
        FONT_SETTING_SCRIPT
    };
};

#endif //UNICOMM_SETTINGMODULE_H