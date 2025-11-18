#ifndef UNICOMM_MARKERSETTINGSCRIPT_H
#define UNICOMM_MARKERSETTINGSCRIPT_H

#include <QScrollArea>

class QComboBox;
class QPushButton;
class QsciScintilla;

class MarkerSettingScript final : public QScrollArea {
    Q_OBJECT

public:
    explicit MarkerSettingScript(QWidget *parent = nullptr);

    void settingImport(const QJsonObject &markerConfigScript) const;

    ~MarkerSettingScript() override = default;

    QJsonObject settingExport() const;

private:
    QStringList m_markerStyleList{};
    QComboBox *m_markerBreakpointStyleComboBox{};
    QPushButton *m_markerBreakpointColorButton{};
    QComboBox *m_markerDebugStyleComboBox{};
    QPushButton *m_markerDebugColorButton{};
    QsciScintilla *m_markerPreviewEditor{};
};

#endif //UNICOMM_MARKERSETTINGSCRIPT_H
