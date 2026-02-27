#include "scriptModule/codeEditor/scintillaWidget.h"

#include "globals.h"

using namespace Scintilla;

// ScintillaWidget public
ScintillaWidget::ScintillaWidget(const QUrl &scriptUrl, QWidget *parent)
    : ScintillaEdit(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    setFrameStyle(NoFrame);
    styleSetBack(STYLE_LINENUMBER, 0xffffff);
}

void ScintillaWidget::fontSet(const QFont &font) {
    styleSetFont(STYLE_DEFAULT, font.family().toUtf8().constData());
    styleSetSize(STYLE_DEFAULT, font.pointSize());
}

void ScintillaWidget::indicatorSet(const int indicator, const QJsonObject &config) const {
    if (config.contains("style")) send(SCI_INDICSETSTYLE, indicator, config["style"].toInt()); // NOLINT
    if (config.contains("fore")) send(SCI_INDICSETFORE, indicator, config["fore"].toInt()); // NOLINT
    if (config.contains("strokeWidth")) send(SCI_INDICSETSTROKEWIDTH, indicator, config["strokeWidth"].toInt()); // NOLINT
    // if (config.contains("alpha")) send(SCI_INDICSETALPHA, indicator, config["alpha"].toInt()); // NOLINT
    // if (config.contains("outlineAlpha")) send(SCI_INDICSETOUTLINEALPHA, indicator, config["outlineAlpha"].toInt()); // NOLINT
    if (config.contains("setUnder")) send(SCI_INDICSETUNDER, indicator, config["setUnder"].toBool()); // NOLINT
    if (config.contains("hoverStyle")) send(SCI_INDICSETHOVERSTYLE, indicator, config["hoverStyle"].toInt()); // NOLINT
    if (config.contains("hoverFore")) send(SCI_INDICSETHOVERFORE, indicator, config["hoverFore"].toInt()); // NOLINT
    // if (config.contains("flags")) send(SCI_INDICSETFLAGS, indicator, config["flags"].toInt()); // NOLINT
}

void ScintillaWidget::marginSet(const int margin, const QJsonObject &config) const {
    if (config.contains("type")) send(SCI_SETMARGINTYPEN, margin, config["type"].toInt()); // NOLINT
    if (config.contains("width")) send(SCI_SETMARGINWIDTHN, margin, config["width"].toInt()); // NOLINT
    // if (config.contains("mask")) send(SCI_SETMARGINMASKN, margin, config["mask"].toInt()); // NOLINT
    if (config.contains("sensitive")) send(SCI_SETMARGINSENSITIVEN, margin, config["sensitive"].toBool()); // NOLINT
    // if (config.contains("cursor")) send(SCI_SETMARGINCURSORN, margin, config["cursor"].toInt()); // NOLINT
    if (config.contains("back")) send(SCI_SETMARGINBACKN, margin, config["back"].toInt()); // NOLINT
    // if (config.contains("left")) send(SCI_SETMARGINLEFT, margin, config["left"].toInt()); // NOLINT
    // if (config.contains("right")) send(SCI_SETMARGINRIGHT, margin, config["right"].toInt()); // NOLINT
    // if (config.contains("color")) send(SCI_SETFOLDMARGINCOLOUR, margin, config["color"].toInt()); // NOLINT
    // if (config.contains("hiColour")) send(SCI_SETFOLDMARGINHICOLOUR, margin, config["hiColour"].toInt()); // NOLINT
    // if (config.contains("text")) send(SCI_MARGINSETTEXT, margin, reinterpret_cast<sptr_t>(config["text"].toString().toUtf8().constData())); // NOLINT
    // if (config.contains("style")) send(SCI_MARGINSETSTYLE, margin, config["style"].toInt()); // NOLINT
    // if (config.contains("styleOffset")) send(SCI_MARGINSETSTYLEOFFSET, margin); // NOLINT
    // if (config.contains("options")) send(SCI_SETMARGINOPTIONS, margin, config["options"].toInt()); // NOLINT
}

void ScintillaWidget::markerSet(const int marker, const QJsonObject &config) const {
    if (config.contains("symbol")) send(SCI_MARKERDEFINE, marker, config["style"].toInt()); // NOLINT
    if (config.contains("fore")) send(SCI_MARKERSETFORE, marker, config["fore"].toInt()); // NOLINT
    // if (config.contains("foreTranslucent")) send(SCI_MARKERSETFORETRANSLUCENT, marker, config["foreTranslucent"].toInt()); // NOLINT
    if (config.contains("back")) send(SCI_MARKERSETBACK, marker, config["back"].toInt()); // NOLINT
    // if (config.contains("backTranslucent")) send(SCI_MARKERSETBACKTRANSLUCENT, marker, config["backTranslucent"].toInt()); // NOLINT
    // if (config.contains("backSelected")) send(SCI_MARKERSETBACKSELECTED, marker, config["backSelected"].toInt()); // NOLINT
    // if (config.contains("backSelectedTranslucent")) send(SCI_MARKERSETBACKSELECTEDTRANSLUCENT, marker, config["backSelectedTranslucent"].toInt()); // NOLINT
    // if (config.contains("strokeWidth")) send(SCI_MARKERSETSTROKEWIDTH, marker, config["strokeWidth"].toInt()); // NOLINT
    // if (config.contains("enableHighlight")) send(SCI_MARKERENABLEHIGHLIGHT, marker, config["enableHighlight"].toBool()); // NOLINT
    // if (config.contains("layer")) send(SCI_MARKERSETLAYER, marker, config["layer"].toInt()); // NOLINT
    // if (config.contains("alpha")) send(SCI_MARKERSETALPHA, marker, config["alpha"].toInt()); // NOLINT
}

void ScintillaWidget::markerAdd(const int marker, const int line, const int time) const {
    send(SCI_MARKERADD, line, marker); // NOLINT
    if (time != -1) {

    }
}

void ScintillaWidget::markerDelete(const int marker, const int line) const {
    send(SCI_MARKERDELETE, line, marker); // NOLINT
}
