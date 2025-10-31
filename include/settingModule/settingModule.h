#ifndef UNICOMM_SETTINGMODULE_H
#define UNICOMM_SETTINGMODULE_H

#include <QDialog>

class QStackedWidget;
class QTreeView;

class LogFontSetting;

class SettingModule final : public QDialog {
    Q_OBJECT

public:
    explicit SettingModule(QWidget *parent = nullptr);

    ~SettingModule() override = default;

    void settingImport(const QJsonObject &settingConfig) const;

private:
    void settingSave();

    QStackedWidget *m_settingStackedWidget{};
    LogFontSetting *m_logFontSettingWidget{};

    enum {
        BLANKSETTING,
        LOGFONTSETTING
    };
};

#endif //UNICOMM_SETTINGMODULE_H