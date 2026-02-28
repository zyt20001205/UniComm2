#include "scriptModule/codeEditor/scintillaWidget.h"

#include <QTimer>

#include "globals.h"

using namespace Scintilla;

// ScintillaWidget public
ScintillaWidget::ScintillaWidget(const QUrl &scriptUrl, QWidget *parent)
    : ScintillaEdit(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    setFrameStyle(NoFrame);
    // folders
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
    // history
    send(SCI_SETCHANGEHISTORY,SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS); // NOLINT
    // style
    send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xffffff); // NOLINT
    send(SCI_STYLESETFORE, STYLE_INDENTGUIDE, 0x000000); // NOLINT
    send(SCI_STYLESETBACK, STYLE_FOLDDISPLAYTEXT, 0xe0e0e0); // NOLINT
    // indent
    send(SCI_SETUSETABS, false); // NOLINT
    send(SCI_SETINDENT, 4); // NOLINT
    send(SCI_SETTABINDENTS, true); // NOLINT
    send(SCI_SETBACKSPACEUNINDENTS, true); // NOLINT
    send(SCI_SETINDENTATIONGUIDES, SC_IV_REAL); // NOLINT
    // misc
    send(SCI_SETSCROLLWIDTH, 1); // NOLINT
    send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT
}

void ScintillaWidget::foldLevelSet(const int line, const int level) const {
    send(SCI_SETFOLDLEVEL, line, level); // NOLINT
}

void ScintillaWidget::fontSet(const QFont &font) {
    styleSetFont(STYLE_DEFAULT, font.family().toUtf8().constData());
    styleSetSize(STYLE_DEFAULT, font.pointSize());
}

void ScintillaWidget::indicatorClear(const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    Position start{};
    Position lengthFill{};
    if (startLine != -1) {
        start = positionGet(startLine, startCharacter);
        lengthFill = positionGet(endLine, endCharacter) - start;
    } else {
        lengthFill = send(SCI_GETTEXTLENGTH);
    }
    send(SCI_SETINDICATORCURRENT, type); // NOLINT
    send(SCI_INDICATORCLEARRANGE, start, lengthFill); // NOLINT
}

void ScintillaWidget::indicatorFill(const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) const {
    const Position start = positionGet(startLine, startCharacter);
    const Position lengthFill = positionGet(endLine, endCharacter) - start;
    if (lengthFill <= 0) return;
    send(SCI_SETINDICATORCURRENT, type); // NOLINT
    send(SCI_INDICATORFILLRANGE, start, lengthFill); // NOLINT
    if (time == -1) return;
    QTimer::singleShot(time, [this, type, startLine, startCharacter, endLine, endCharacter] { indicatorClear(type, startLine, startCharacter, endLine, endCharacter); });
}

void ScintillaWidget::indicatorSet(const int type, const QJsonObject &config) const {
    if (config.contains("style")) send(SCI_INDICSETSTYLE, type, config["style"].toInt()); // NOLINT
    if (config.contains("fore")) send(SCI_INDICSETFORE, type, config["fore"].toInt()); // NOLINT
    // if (config.contains("strokeWidth")) send(SCI_INDICSETSTROKEWIDTH, type, config["strokeWidth"].toInt()); // NOLINT
    if (config.contains("alpha")) send(SCI_INDICSETALPHA, type, config["alpha"].toInt()); // NOLINT
    if (config.contains("outlineAlpha")) send(SCI_INDICSETOUTLINEALPHA, type, config["outlineAlpha"].toInt()); // NOLINT
    if (config.contains("setUnder")) send(SCI_INDICSETUNDER, type, config["setUnder"].toBool()); // NOLINT
    // if (config.contains("hoverStyle")) send(SCI_INDICSETHOVERSTYLE, type, config["hoverStyle"].toInt()); // NOLINT
    // if (config.contains("hoverFore")) send(SCI_INDICSETHOVERFORE, type, config["hoverFore"].toInt()); // NOLINT
    // if (config.contains("flags")) send(SCI_INDICSETFLAGS, type, config["flags"].toInt()); // NOLINT
}

int ScintillaWidget::lineCountGet() const {
    return static_cast<int>(send(SCI_GETLINECOUNT));
}

int ScintillaWidget::lineGet(const Position position) const {
    return static_cast<int>(send(SCI_LINEFROMPOSITION, position));
}

void ScintillaWidget::marginSet(const int type, const QJsonObject &config) const {
    if (config.contains("type")) send(SCI_SETMARGINTYPEN, type, config["type"].toInt()); // NOLINT
    if (config.contains("width")) send(SCI_SETMARGINWIDTHN, type, config["width"].toInt()); // NOLINT
    if (config.contains("mask")) send(SCI_SETMARGINMASKN, type, config["mask"].toInt()); // NOLINT
    if (config.contains("sensitive")) send(SCI_SETMARGINSENSITIVEN, type, config["sensitive"].toBool()); // NOLINT
    // if (config.contains("cursor")) send(SCI_SETMARGINCURSORN, type, config["cursor"].toInt()); // NOLINT
    // if (config.contains("back")) send(SCI_SETMARGINBACKN, type, config["back"].toInt()); // NOLINT
    // if (config.contains("left")) send(SCI_SETMARGINLEFT, type, config["left"].toInt()); // NOLINT
    // if (config.contains("right")) send(SCI_SETMARGINRIGHT, type, config["right"].toInt()); // NOLINT
    // if (config.contains("text")) send(SCI_MARGINSETTEXT, type, reinterpret_cast<sptr_t>(config["text"].toString().toUtf8().constData())); // NOLINT
    // if (config.contains("style")) send(SCI_MARGINSETSTYLE, type, config["style"].toInt()); // NOLINT
    // if (config.contains("styleOffset")) send(SCI_MARGINSETSTYLEOFFSET, type); // NOLINT
    // if (config.contains("options")) send(SCI_SETMARGINOPTIONS, type, config["options"].toInt()); // NOLINT
}

void ScintillaWidget::markerAdd(const int type, const int line, const int time) const {
    send(SCI_MARKERADD, line, type); // NOLINT
    send(SCI_ENSUREVISIBLE, line); // NOLINT
    if (time == -1) return;
    QTimer::singleShot(time, [this, type, line] { markerDelete(type, line); });
}

void ScintillaWidget::markerDelete(const int type, const int line) const {
    send(SCI_MARKERDELETE, line, type); // NOLINT
}

int ScintillaWidget::markerGet(const int line) const {
    return static_cast<int>(send(SCI_MARKERGET, line));
}

void ScintillaWidget::markerSet(const int type, const QJsonObject &config) const {
    if (config.contains("symbol")) send(SCI_MARKERDEFINE, type, config["style"].toInt()); // NOLINT
    if (config.contains("fore")) send(SCI_MARKERSETFORE, type, config["fore"].toInt()); // NOLINT
    // if (config.contains("foreTranslucent")) send(SCI_MARKERSETFORETRANSLUCENT, type, config["foreTranslucent"].toInt()); // NOLINT
    if (config.contains("back")) send(SCI_MARKERSETBACK, type, config["back"].toInt()); // NOLINT
    // if (config.contains("backTranslucent")) send(SCI_MARKERSETBACKTRANSLUCENT, type, config["backTranslucent"].toInt()); // NOLINT
    // if (config.contains("backSelected")) send(SCI_MARKERSETBACKSELECTED, type, config["backSelected"].toInt()); // NOLINT
    // if (config.contains("backSelectedTranslucent")) send(SCI_MARKERSETBACKSELECTEDTRANSLUCENT, type, config["backSelectedTranslucent"].toInt()); // NOLINT
    // if (config.contains("strokeWidth")) send(SCI_MARKERSETSTROKEWIDTH, type, config["strokeWidth"].toInt()); // NOLINT
    // if (config.contains("enableHighlight")) send(SCI_MARKERENABLEHIGHLIGHT, type, config["enableHighlight"].toBool()); // NOLINT
    // if (config.contains("layer")) send(SCI_MARKERSETLAYER, type, config["layer"].toInt()); // NOLINT
    // if (config.contains("alpha")) send(SCI_MARKERSETALPHA, type, config["alpha"].toInt()); // NOLINT
}

Position ScintillaWidget::positionGet(const int line, const int character) const {
    return send(SCI_POSITIONRELATIVE, send(SCI_POSITIONFROMLINE, line), character);
}

void ScintillaWidget::savepointSet() const {
    send(SCI_SETSAVEPOINT); // NOLINT
}

QHash<QString, int> ScintillaWidget::selectionGet() const {
    const Position position = send(SCI_GETCURRENTPOS);
    const int line = static_cast<int>(send(SCI_LINEFROMPOSITION, position));
    const int character = static_cast<int>(send(SCI_GETCOLUMN, position));
    const Position startPosition = send(SCI_GETSELECTIONSTART);
    const Position endPosition = send(SCI_GETSELECTIONEND);
    const int characters = static_cast<int>(send(SCI_COUNTCHARACTERS, startPosition, endPosition));
    const int startLine = static_cast<int>(send(SCI_LINEFROMPOSITION, startPosition));
    const int endLine = static_cast<int>(send(SCI_LINEFROMPOSITION, endPosition));
    const int lines = endLine - startLine;
    return QHash<QString, int>{
        {"line", line},
        {"character", character},
        {"lines", lines},
        {"characters", characters}
    };
}

void ScintillaWidget::textSet(const QString &text) const {
    send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(text.toUtf8().constData())); // NOLINT
}
