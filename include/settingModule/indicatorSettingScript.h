#ifndef UNICOMM_INDICATORSETTINGSCRIPT_H
#define UNICOMM_INDICATORSETTINGSCRIPT_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QsciScintilla;

class IndicatorSettingScript final : public QWidget {
    Q_OBJECT

public:
    explicit IndicatorSettingScript(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &indicatorConfigScript) const;

    ~IndicatorSettingScript() override = default;

    QJsonObject settingExport() const;

private:
    QStringList m_indicatorStyleList{};
    QComboBox *m_indicatorErrorStyleComboBox{};
    QPushButton *m_indicatorErrorColorButton{};
    QComboBox *m_indicatorWarningStyleComboBox{};
    QPushButton *m_indicatorWarningColorButton{};
    QComboBox *m_indicatorInfoStyleComboBox{};
    QPushButton *m_indicatorInfoColorButton{};
    QComboBox *m_indicatorHintStyleComboBox{};
    QPushButton *m_indicatorHintColorButton{};
    QsciScintilla *m_diagnosticPreviewEditor{};
};

#endif //UNICOMM_INDICATORSETTINGSCRIPT_H
