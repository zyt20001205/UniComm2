#ifndef UNICOMM_INDICATORSETTINGSCRIPT_H
#define UNICOMM_INDICATORSETTINGSCRIPT_H

#include <QScrollArea>

class QComboBox;
class QPushButton;
class QsciScintilla;

class IndicatorSettingScript final : public QScrollArea {
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
    QComboBox *m_indicatorSearchStyleComboBox{};
    QPushButton *m_indicatorSearchColorButton{};
    QComboBox *m_indicatorHighlightStyleComboBox{};
    QPushButton *m_indicatorHighlightColorButton{};
    QsciScintilla *m_searchPreviewEditor{};
};

#endif //UNICOMM_INDICATORSETTINGSCRIPT_H
