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
    // diagnostic
    QComboBox *m_indicatorErrorStyleComboBox{};
    QPushButton *m_indicatorErrorColorButton{};
    QComboBox *m_indicatorWarningStyleComboBox{};
    QPushButton *m_indicatorWarningColorButton{};
    QComboBox *m_indicatorInfoStyleComboBox{};
    QPushButton *m_indicatorInfoColorButton{};
    QComboBox *m_indicatorHintStyleComboBox{};
    QPushButton *m_indicatorHintColorButton{};
    QsciScintilla *m_diagnosticPreviewEditor{};
    // highlight
    QComboBox *m_indicatorHighlightStyleComboBox{};
    QPushButton *m_indicatorHighlightColorButton{};
    QComboBox *m_indicatorReadStyleComboBox{};
    QPushButton *m_indicatorReadColorButton{};
    QComboBox *m_indicatorWriteStyleComboBox{};
    QPushButton *m_indicatorWriteColorButton{};
    QsciScintilla *m_highlightPreviewEditor{};
    // misc
    QComboBox *m_indicatorSearchStyleComboBox{};
    QPushButton *m_indicatorSearchColorButton{};
    QComboBox *m_indicatorSelectionStyleComboBox{};
    QPushButton *m_indicatorSelectionColorButton{};
    QsciScintilla *m_searchPreviewEditor{};
};

#endif //UNICOMM_INDICATORSETTINGSCRIPT_H
