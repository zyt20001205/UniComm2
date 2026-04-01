#include "scriptModule/codeEditor/scintillaWidget.h"

#include <QFile>
#include <QTimer>

#include "globals.h"

using namespace Scintilla;

// public
ScintillaWidget::ScintillaWidget(QWidget *parent)
    : ScintillaEdit(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    setFrameStyle(NoFrame);
}

// public: file
int ScintillaWidget::codePageGet() const {
    return static_cast<int>(send(SCI_GETCODEPAGE));
}

int ScintillaWidget::eolModeGet() const {
    return static_cast<int>(send(SCI_GETEOLMODE));
}

void ScintillaWidget::eolModeSet(const int eolMode) const {
    send(SCI_SETEOLMODE, eolMode); // NOLINT
    send(SCI_CONVERTEOLS, eolMode); // NOLINT
}

bool ScintillaWidget::eolViewGet() const {
    return send(SCI_GETVIEWEOL);
}

void ScintillaWidget::eolViewSet(const bool status) const {
    send(SCI_SETVIEWEOL, status); // NOLINT
}

bool ScintillaWidget::modifyGet() const {
    return send(SCI_GETMODIFY);
}

void ScintillaWidget::readonlySet(const bool status) const {
    send(SCI_SETREADONLY, status); // NOLINT
}

bool ScintillaWidget::readonlyGet() const {
    return send(SCI_GETREADONLY);
}

void ScintillaWidget::savepointSet() const {
    send(SCI_SETSAVEPOINT); // NOLINT
}

// public: annotation
void ScintillaWidget::annotationClear() const {
    send(SCI_ANNOTATIONCLEARALL); // NOLINT
}

void ScintillaWidget::annotationSet(const int line, const QString &annotation) const {
    send(SCI_ANNOTATIONSETTEXT, line, reinterpret_cast<sptr_t>(annotation.toUtf8().constData()));
    send(SCI_ANNOTATIONSETSTYLE, line, STYLE_ANNOTATION); // NOLINT
}

void ScintillaWidget::eolAnnotationClear() const {
    send(SCI_EOLANNOTATIONCLEARALL); // NOLINT
}

void ScintillaWidget::eolAnnotationSet(const int line, const QString &annotation) const {
    send(SCI_EOLANNOTATIONSETTEXT, line, reinterpret_cast<sptr_t>(annotation.toUtf8().constData()));
}

void ScintillaWidget::focusSet(const bool status) const {
    send(SCI_SETFOCUS, status); // NOLINT
}

// public: fold
void ScintillaWidget::foldLevelSet(const int line, const int level) const {
    send(SCI_SETFOLDLEVEL, line, level); // NOLINT
}

void ScintillaWidget::foldContractTop() const {
    send(SCI_FOLDALL, SC_FOLDACTION_CONTRACT); // NOLINT
}

void ScintillaWidget::foldContractRecursively() const {
    send(SCI_FOLDALL, SC_FOLDACTION_CONTRACT | SC_FOLDACTION_CONTRACT_EVERY_LEVEL); // NOLINT
}

void ScintillaWidget::foldExpandRecursively() const {
    send(SCI_FOLDALL, SC_FOLDACTION_EXPAND); // NOLINT
}

// public: font
void ScintillaWidget::fontSet(const QFont &font) {
    styleSetFont(STYLE_DEFAULT, font.family().toUtf8().constData());
    styleSetSize(STYLE_DEFAULT, font.pointSize());
}

// public: height
int ScintillaWidget::heightGet() const {
    return static_cast<int>(send(SCI_TEXTHEIGHT, 0));
}

// public: index
QHash<QString, int> ScintillaWidget::indexGet(Position position) const {
    if (position == -1) position = positionGet();
    const int line = static_cast<int>(send(SCI_LINEFROMPOSITION, position));
    const int character = static_cast<int>(send(SCI_GETCOLUMN, position));
    return QHash<QString, int>{
        {"line", line},
        {"character", character}
    };
}

QHash<QString, int> ScintillaWidget::wordIndexGet(const Position position, const bool onlyWordCharacters) const {
    const Position startPosition = send(SCI_WORDSTARTPOSITION, position, onlyWordCharacters);
    const Position endPosition = send(SCI_WORDENDPOSITION, position, onlyWordCharacters);
    const auto startIndex = indexGet(startPosition);
    const auto endIndex = indexGet(endPosition);
    return QHash<QString, int>{
        {"startLine", startIndex["line"]},
        {"startCharacter", startIndex["character"]},
        {"endLine", endIndex["line"]},
        {"endCharacter", endIndex["character"]}
    };
}

QHash<QString, int> ScintillaWidget::wordIndexGet(const int line, const int character, const bool onlyWordCharacters) const {
    const auto position = positionGet(line, character);
    return wordIndexGet(position, onlyWordCharacters);
}

void ScintillaWidget::indexSet(const int line, const int character) const {
    const auto position = positionGet(line, character);
    positionSet(position);
}

// public: indicator
void ScintillaWidget::indicatorDefine(const int type, const QJsonObject &config) const {
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

void ScintillaWidget::indicatorFill(const int type, const int startLine, const int startCharacter, const int endLine, const int endCharacter, const int time) const {
    const Position start = positionGet(startLine, startCharacter);
    const Position length = positionGet(endLine, endCharacter) - start;
    if (length <= 0) return;
    send(SCI_SETINDICATORCURRENT, type); // NOLINT
    send(SCI_INDICATORFILLRANGE, start, length); // NOLINT
    if (time == -1) return;
    QTimer::singleShot(time, [this, type, startLine, startCharacter, endLine, endCharacter] { indicatorClear(type, startLine, startCharacter, endLine, endCharacter); });
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

int ScintillaWidget::indicatorGet(const Position position) const {
    return send(SCI_INDICATORALLONFOR, position);
}

// public: length
Position ScintillaWidget::lengthGet() const {
    return send(SCI_GETLENGTH);
}

// public: line
int ScintillaWidget::lineCountGet() const {
    return static_cast<int>(send(SCI_GETLINECOUNT));
}

void ScintillaWidget::lineDuplicate() const {
    send(SCI_LINEDUPLICATE); // NOLINT
}

int ScintillaWidget::lineGet(const Position position) const {
    return static_cast<int>(send(SCI_LINEFROMPOSITION, position));
}

// public: margin
void ScintillaWidget::marginDefine(const int type, const QJsonObject &config) const {
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

void ScintillaWidget::marginTextSet(const int line, const QString &text) const {
    send(SCI_MARGINSETTEXT, line, reinterpret_cast<sptr_t>(text.toUtf8().constData())); // NOLINT
}

int ScintillaWidget::marginCountGet() const {
    return send(SCI_GETMARGINS); // NOLINT
}

int ScintillaWidget::marginWidthGet(const int col) const {
    if (col == -1) {
        int width = 0;
        for (int current = 0; current < marginCountGet(); ++current) {
            width += send(SCI_GETMARGINWIDTHN, current);
        }
        return width;
    }
    return static_cast<int>(send(SCI_GETMARGINWIDTHN, col));
}

// public: marker
void ScintillaWidget::markerDefine(const int type, const QJsonObject &config) const {
    if (config.contains("symbol")) send(SCI_MARKERDEFINE, type, config["symbol"].toInt()); // NOLINT
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

void ScintillaWidget::markerAdd(const int type, const int line, const int time) const {
    send(SCI_MARKERADD, line, type); // NOLINT
    send(SCI_ENSUREVISIBLE, line); // NOLINT
    if (time == -1) return;
    QTimer::singleShot(time, [this, type, line] { markerDelete(type, line); });
}

void ScintillaWidget::markerDelete(const int type, const int line) const {
    if (line == -1) send(SCI_MARKERDELETEALL, type); // NOLINT
    else send(SCI_MARKERDELETE, line, type); // NOLINT
}

int ScintillaWidget::markerGet(const int line) const {
    return static_cast<int>(send(SCI_MARKERGET, line));
}

// public: point
QHash<QString, int> ScintillaWidget::pointGet(const int line, const int character) const {
    const auto position = positionGet(line, character);
    const int x = static_cast<int>(send(SCI_POINTXFROMPOSITION, 0, position));
    const int y = static_cast<int>(send(SCI_POINTYFROMPOSITION, 0, position));
    return QHash<QString, int>{
        {"x", x},
        {"y", y}
    };
}

// public: position
Position ScintillaWidget::positionGet(const int line, const int character) const {
    if (line == -1) {
        return send(SCI_GETCURRENTPOS);
    }
    if (character == -1) {
        return send(SCI_GETLINEENDPOSITION, line);
    }
    return send(SCI_POSITIONRELATIVE, send(SCI_POSITIONFROMLINE, line), character);
}

Position ScintillaWidget::positionGet(const QPoint &point) const {
    return send(SCI_POSITIONFROMPOINT, point.x(), point.y());
}

Position ScintillaWidget::closePositionGet(const QPoint &point) const {
    return send(SCI_POSITIONFROMPOINTCLOSE, point.x(), point.y());
}

void ScintillaWidget::positionSet(const Position position) const {
    send(SCI_GOTOPOS, position); // NOLINT
}

// public: search
void ScintillaWidget::searchFlagsSet(const bool matchCase, const bool wholeWord, const bool wordStart, const bool regExp) const {
    auto flags = SCFIND_NONE;
    if (matchCase) flags |= SCFIND_MATCHCASE;
    if (wholeWord) flags |= SCFIND_WHOLEWORD;
    if (wordStart) flags |= SCFIND_WORDSTART;
    if (regExp) flags |= SCFIND_REGEXP;
    send(SCI_SETSEARCHFLAGS, flags); // NOLINT
}

Position ScintillaWidget::targetGetStart() const {
    return send(SCI_GETTARGETSTART);
}

void ScintillaWidget::targetSetStart(const Position position) const {
    send(SCI_SETTARGETSTART, position); // NOLINT
}

Position ScintillaWidget::targetGetEnd() const {
    return send(SCI_GETTARGETEND);
}

void ScintillaWidget::targetSetEnd(const Position position) const {
    send(SCI_SETTARGETEND, position); // NOLINT
}

void ScintillaWidget::targetSetWhole() const {
    send(SCI_TARGETWHOLEDOCUMENT); // NOLINT
}

Position ScintillaWidget::targetSearch(const QString &text) const {
    return send(SCI_SEARCHINTARGET, text.length(), reinterpret_cast<uptr_t>(text.toUtf8().constData())); // NOLINT
}

// public: selection
QHash<QString, int> ScintillaWidget::selectionGet() const {
    const auto index = indexGet(positionGet());
    const Position startPosition = send(SCI_GETSELECTIONSTART);
    const Position endPosition = send(SCI_GETSELECTIONEND);
    const int characters = static_cast<int>(send(SCI_COUNTCHARACTERS, startPosition, endPosition));
    const auto startIndex = indexGet(startPosition);
    const auto endIndex = indexGet(endPosition);
    return QHash<QString, int>{
        {"line", index["line"]},
        {"character", index["character"]},
        {"startLine", startIndex["line"]},
        {"startCharacter", startIndex["character"]},
        {"startPosition", startPosition},
        {"endLine", endIndex["line"]},
        {"endCharacter", endIndex["character"]},
        {"lines", endIndex["line"] - startIndex["line"]},
        {"characters", characters}
    };
}

void ScintillaWidget::selectionSet(const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    const Position anchor = positionGet(startLine, startCharacter);
    const Position caret = positionGet(endLine, endCharacter);
    send(SCI_SETSEL, anchor, caret); // NOLINT
}

// public: style
void ScintillaWidget::styleDefine(const int type, const QJsonObject &config) const {
    // if (config.contains("bold")) send(SCI_STYLESETBOLD, type, config["bold"].toBool()); // NOLINT
    // if (config.contains("weight")) send(SCI_STYLESETWEIGHT, type, config["weight"].toInt()); // NOLINT
    // if (config.contains("stretch")) send(SCI_STYLESETSTRETCH, type, config["stretch"].toInt()); // NOLINT
    // if (config.contains("italic")) send(SCI_STYLESETITALIC, type, config["italic"].toBool()); // NOLINT
    // if (config.contains("underline")) send(SCI_STYLESETUNDERLINE, type, config["underline"].toBool()); // NOLINT
    if (config.contains("fore")) send(SCI_STYLESETFORE, type, config["fore"].toInt()); // NOLINT    if (config.contains("fore")) send(SCI_STYLESETFORE, type, config["fore"].toInt()); // NOLINT
    // if (config.contains("back")) send(SCI_STYLESETBACK, type, config["back"].toInt()); // NOLINT
    // if (config.contains("eolFilled")) send(SCI_STYLESETEOLFILLED, type, config["eolFilled"].toBool()); // NOLINT
    // if (config.contains("characterSet")) send(SCI_STYLESETCHARACTERSET, type, config["characterSet"].toInt()); // NOLINT
    // if (config.contains("case")) send(SCI_STYLESETCASE, type, config["case"].toInt()); // NOLINT
    // if (config.contains("visible")) send(SCI_STYLESETVISIBLE, type, config["visible"].toBool()); // NOLINT
    // if (config.contains("changeable")) send(SCI_STYLESETCHANGEABLE, type, config["changeable"].toBool()); // NOLINT
    // if (config.contains("hotspot")) send(SCI_STYLESETHOTSPOT, type, config["hotspot"].toBool()); // NOLINT
    // if (config.contains("checkMonospaced")) send(SCI_STYLESETCHECKMONOSPACED, type, config["checkMonospaced"].toBool()); // NOLINT
    // if (config.contains("representation")) send(SCI_STYLESETINVISIBLEREPRESENTATION, type, reinterpret_cast<sptr_t>(config["bold"].toString().toUtf8().constData())); // NOLINT
    // if (config.contains("locale")) send(SCI_SETFONTLOCALE, type, reinterpret_cast<sptr_t>(config["locale"].toString().toUtf8().constData())); // NOLINT
}

int ScintillaWidget::styleGet(const Position position) const {
    return send(SCI_GETSTYLEAT, position); // NOLINT
}

void ScintillaWidget::styleSet(const int type, const int startLine, const int startCharacter, int length) const {
    Position start{};
    if (startLine != -1) {
        start = positionGet(startLine, startCharacter);
    } else {
        length = static_cast<int>(lengthGet());
    }
    // TODO: safety check
    // if (startPos < 0 || endPos > m_editorWidget->length() || length <= 0) {
    //     qDebug() << "long string skipped" << currentLine << currentChar;
    //     continue;
    // }
    send(SCI_STARTSTYLING, start); // NOLINT
    send(SCI_SETSTYLING, length, type); // NOLINT
}

// public: text
void ScintillaWidget::textAppend(const QString &text) const {
    const auto ba = text.toUtf8();
    send(SCI_APPENDTEXT, ba.length(), reinterpret_cast<sptr_t>(ba.constData()));
}

void ScintillaWidget::textClear() const {
    send(SCI_CLEARALL); // NOLINT
}

QString ScintillaWidget::textGet(const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    Position startPosition{};
    Position endPosition{};
    Position length{};
    if (startLine == -1) {
        endPosition = lengthGet();
        length = lengthGet();
    } else {
        startPosition = positionGet(startLine, startCharacter);
        endPosition = positionGet(endLine, endCharacter);
        length = endPosition - startPosition;
    }
    QByteArray buffer(static_cast<int>(length + 1), '\0');
    Sci_TextRange tr = {{static_cast<int>(startPosition), static_cast<int>(endPosition)}, buffer.data()};
    send(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
    buffer.chop(1); // Remove extra NUL
    return QString::fromUtf8(buffer.constData(), static_cast<int>(length));
}

QString ScintillaWidget::textGetSelected() const {
    const int length = static_cast<int>(send(SCI_GETSELTEXT, 0, NULL));
    QByteArray buffer(length + 1, '\0');
    send(SCI_GETSELTEXT, 0, reinterpret_cast<sptr_t>(buffer.data()));
    buffer.chop(1); // Remove extra NUL
    return QString::fromUtf8(buffer.constData(), length);
}

void ScintillaWidget::textSet(const QString &text, const int startLine, const int startCharacter, const int endLine, const int endCharacter) const {
    if (startLine == -1) {
        send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(text.toUtf8().constData())); // NOLINT
    } else {
        selectionSet(startLine, startCharacter, endLine, endCharacter);
        textSetSelected(text);
    }
}

void ScintillaWidget::textSetSelected(const QString &text) const {
    send(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(text.toUtf8().constData()));
}
