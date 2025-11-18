#ifndef UNICOMM_SETTINGMODULE_H
#define UNICOMM_SETTINGMODULE_H

#include <QDialog>
#include <QJsonObject>

class QStackedWidget;
class QTreeView;

class FontSettingLog;
class FontSettingScript;
class IndicatorSettingScript;
class MarkerSettingScript;

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

    void reloadScriptIndicator(const QJsonObject &indicatorConfigScript);

    void saveScriptIndicator(const QJsonObject &indicatorConfigScript);

    void reloadScriptMarker(const QJsonObject &markerConfigScript);

    void saveScriptMarker(const QJsonObject &markerConfigScript);

private:
    void settingApply();

    void settingCancel();

    void settingSave();

    QStackedWidget *m_settingStackedWidget{};
    FontSettingLog *m_fontSettingLogWidget{};
    FontSettingScript *m_fontSettingScriptWidget{};
    IndicatorSettingScript *m_indicatorSettingScriptWidget{};
    MarkerSettingScript *m_markerSettingScriptWidget{};

    enum {
        BLANK_SETTING,
        FONT_SETTING_LOG,
        FONT_SETTING_SCRIPT,
        INDICATOR_SETTING_SCRIPT,
        MARKER_SETTING_SCRIPT
    };
};

#endif //UNICOMM_SETTINGMODULE_H