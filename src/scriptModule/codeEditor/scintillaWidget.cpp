#include "scriptModule/codeEditor/scintillaWidget.h"

#include <QTimer>

#include "globals.h"

using namespace Scintilla;

// ScintillaWidget public
ScintillaWidget::ScintillaWidget(const QUrl &scriptUrl, QWidget *parent)
    : ScintillaEdit(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    setFrameStyle(NoFrame);
    // folding
    send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1")); // NOLINT
    send(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS); // NOLINT
    send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS); // NOLINT
    for (int i = SC_MARKNUM_FOLDEREND; i <= SC_MARKNUM_FOLDEROPEN; ++i) {
        send(SCI_MARKERSETFORE, i, 0xffffff); // NOLINT
        send(SCI_MARKERSETBACK, i, 0x000000); // NOLINT
    }
    send(SCI_SETFOLDMARGINCOLOUR, true, 0xffffff); // NOLINT
    send(SCI_SETFOLDMARGINHICOLOUR, true, 0xffffff); // NOLINT
    send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
    send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
    // style
    send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xffffff); // NOLINT
    send(SCI_STYLESETBACK, STYLE_FOLDDISPLAYTEXT, 0xeeeeee); // NOLINT
    // misc
    send(SCI_SETTABWIDTH, 4); // NOLINT
}

void ScintillaWidget::foldLevelSet(const int line, const int level) const {
    send(SCI_SETFOLDLEVEL, line, level); // NOLINT
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

int ScintillaWidget::lineCountGet() const {
    return static_cast<int>(send(SCI_GETLINECOUNT));
}

int ScintillaWidget::lineGet(const Position position) const {
    return send(SCI_LINEFROMPOSITION, position);
}

void ScintillaWidget::marginSet(const int margin, const QJsonObject &config) const {
    if (config.contains("type")) send(SCI_SETMARGINTYPEN, margin, config["type"].toInt()); // NOLINT
    if (config.contains("width")) send(SCI_SETMARGINWIDTHN, margin, config["width"].toInt()); // NOLINT
    if (config.contains("mask")) send(SCI_SETMARGINMASKN, margin, config["mask"].toInt()); // NOLINT
    if (config.contains("sensitive")) send(SCI_SETMARGINSENSITIVEN, margin, config["sensitive"].toBool()); // NOLINT
    // if (config.contains("cursor")) send(SCI_SETMARGINCURSORN, margin, config["cursor"].toInt()); // NOLINT
    if (config.contains("back")) send(SCI_SETMARGINBACKN, margin, config["back"].toInt()); // NOLINT
    // if (config.contains("left")) send(SCI_SETMARGINLEFT, margin, config["left"].toInt()); // NOLINT
    // if (config.contains("right")) send(SCI_SETMARGINRIGHT, margin, config["right"].toInt()); // NOLINT
    // if (config.contains("text")) send(SCI_MARGINSETTEXT, margin, reinterpret_cast<sptr_t>(config["text"].toString().toUtf8().constData())); // NOLINT
    // if (config.contains("style")) send(SCI_MARGINSETSTYLE, margin, config["style"].toInt()); // NOLINT
    // if (config.contains("styleOffset")) send(SCI_MARGINSETSTYLEOFFSET, margin); // NOLINT
    // if (config.contains("options")) send(SCI_SETMARGINOPTIONS, margin, config["options"].toInt()); // NOLINT
}

void ScintillaWidget::markerAdd(const int marker, const int line, const int time) const {
    send(SCI_MARKERADD, line, marker); // NOLINT
    send(SCI_ENSUREVISIBLE, line); // NOLINT
    if (time == -1) return;
    QTimer::singleShot(time, [this, marker, line] {markerDelete(marker, line);});
}

void ScintillaWidget::markerDelete(const int marker, const int line) const {
    send(SCI_MARKERDELETE, line, marker); // NOLINT
}

int ScintillaWidget::markerGet(const int line) const {
    return send(SCI_MARKERGET, line);
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