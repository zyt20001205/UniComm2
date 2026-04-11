// #include "document/module/editorWidget.h"
//
// #include <QMessageBox>
//
// #include "globals.h"
//
// // EditorWidget public
// EditorWidget::EditorWidget(const QUrl &scriptUrl, QWidget *parent)
//     : QsciScintilla(parent),
//       m_scriptUrl(scriptUrl),
//       m_autoPairHash{
//           {'(', ')'},
//           {'[', ']'},
//           {'{', '}'},
//           {'"', '"'},
//           {'\'', '\''},
//       },
//       m_dwellTimer(new QTimer(this)),
//       m_typeTimer(new QTimer(this)) {
//     setContextMenuPolicy(Qt::NoContextMenu);
//     setFrameStyle(NoFrame);
//     // set markers
//     markerDefine(RightTriangle, MARKER_REGION);
//     setMarkerBackgroundColor(Qt::cyan, MARKER_REGION);
//     setMarkerForegroundColor(Qt::cyan, MARKER_REGION);
//
//     markerDefine(Background, MARKER_ERROR);
//     setMarkerBackgroundColor(QColor(255, 230, 230), MARKER_ERROR);
//
//     markerDefine(Background, MARKER_HINT);
//     setMarkerBackgroundColor(Qt::cyan, MARKER_HINT);
//
//     // set margins
//     setMarginType(0, NumberMargin);
//     SendScintilla(SCI_STYLESETBACK, STYLE_LINENUMBER, QColor(Qt::white));
//     QsciScintilla::setMarginWidth(0, 32);
//
//     setMarginType(1, SymbolMargin);
//     QsciScintilla::setMarginWidth(1, 16);
//     QsciScintilla::setMarginSensitivity(1, true);
//
//     setMarginType(2, SymbolMargin);
//     SendScintilla(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS); // NOLINT
//     SendScintilla(SCI_SETFOLDMARGINHICOLOUR, true, QColor(Qt::white));
//     SendScintilla(SCI_SETFOLDMARGINCOLOUR, true, QColor(Qt::white));
//     QsciScintilla::setMarginWidth(2, 16);
//     QsciScintilla::setMarginSensitivity(2, true);
//     QsciScintilla::setFolding(CircledTreeFoldStyle);
//
//     setMarginType(3, SymbolMarginColor);
//     setMarginBackgroundColor(3, QColor(Qt::black));
//     QsciScintilla::setMarginWidth(3, 1);
//     // set styles !!!color format is BGR!!!
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_NAMESPACE, static_cast<long>(0x808000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_CLASS, static_cast<long>(0x808000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_TYPE, static_cast<long>(0xB33300)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_PARAMETER, static_cast<long>(0x000000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_VARIABLE, static_cast<long>(0x000000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_PROPERTY, static_cast<long>(0x7A0E66)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_ENUMMEMBAER, static_cast<long>(0x941087)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_FUNCTION_DECLARATION, static_cast<long>(0x7A6200)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_FUNCTION_CALL, static_cast<long>(0x000000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_METHOD, static_cast<long>(0x000000)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_MACRO, static_cast<long>(0x2E541F)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETBOLD, LUA_TOKEN_MACRO, 1); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_KEYWORD, static_cast<long>(0xB33300)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_COMMENT, static_cast<long>(0x8C8C8C)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_STRING, static_cast<long>(0x177D06)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_NUMBER, static_cast<long>(0xEB5017)); // NOLINT
//     SendScintilla(QsciScintillaBase::SCI_STYLESETFORE, LUA_TOKEN_OPERATOR, static_cast<long>(0x000000)); // NOLINT
//     // script scintilla settings
//     QsciScintilla::setBraceMatching(SloppyBraceMatch);
//     QsciScintilla::setBackspaceUnindents(true);
//     QsciScintilla::setEolMode(EolWindows);
//     // QsciScintilla::setEolVisibility(true);
//     QsciScintilla::setIndentationGuides(true);
//     QsciScintilla::setTabWidth(4);
//     setScrollWidth(1);
//     setScrollWidthTracking(true);
//     // connect
//     connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(pairHandle(int)));
//     connect(this, SIGNAL(textChanged()), this, SLOT(typeHandle()));
//     // init timer
//     m_dwellTimer->setSingleShot(true);
//     m_dwellTimer->setInterval(1000);
//     connect(m_dwellTimer, &QTimer::timeout, this, &EditorWidget::dwellHandle);
//     m_typeTimer->setInterval(500);
//     m_typeTimer->setSingleShot(true);
//     connect(m_typeTimer, &QTimer::timeout, this, &EditorWidget::requestIdle);
//     // load breakpoints
//     QTimer::singleShot(0, this, [this] {
//         breakpointLoad();
//         // regionLoad();
//     });
// }
//
// void EditorWidget::breakpointLoad() {
//     if (g_breakpoints.contains(m_scriptUrl)) {
//         for (const auto &line: g_breakpoints[m_scriptUrl].keys()) {
//             markerInsert(MARKER_BREAKPOINT_ENABLED, line - 1);
//         }
//     }
// }
//
// TODO: To be added in the near future!
// void EditorWidget::regionLoad() {
//     for (int line = 1; line < lines(); ++line) {
//         const QString lineText = text(line);
//         if (lineText.contains("--#region")) {
//             markerInsert(MARKER_REGION, line);
//         }
//     }
// }
//
// void EditorWidget::textSearch(const QString &text, const int flag) {
//     // clear previous search result
//     m_searchText = text;
//     m_searchFlag = flag;
//     m_currentIndex = 0;
//     m_searchList.clear();
//     const int docLength = SendScintilla(SCI_GETLENGTH);
//     SendScintilla(SCI_SETINDICATORCURRENT, INDICATOR_SEARCH); // NOLINT
//     SendScintilla(SCI_INDICATORCLEARRANGE, 0, docLength); // NOLINT
//     // start searching
//     if (!text.isEmpty()) {
//         SendScintilla(SCI_GOTOPOS, 0); // NOLINT
//         SendScintilla(SCI_SEARCHANCHOR); // NOLINT
//         int count = 0;
//         while (SendScintilla(SCI_SEARCHNEXT, flag, text.toUtf8().constData()) != -1) {
//             const int start = SendScintilla(SCI_GETSELECTIONSTART);
//             const int end = SendScintilla(SCI_GETSELECTIONEND);
//             const int length = end - start;
//             m_searchList.append({start, end, length});
//             SendScintilla(SCI_SETINDICATORCURRENT, INDICATOR_SEARCH); // NOLINT
//             SendScintilla(SCI_INDICATORFILLRANGE, start, length); // NOLINT
//             SendScintilla(SCI_GOTOPOS, end); // NOLINT
//             SendScintilla(SCI_SEARCHANCHOR); // NOLINT
//             count++;
//         }
//     }
//     searchHandle();
// }
//
// void EditorWidget::prevSearch() {
//     // go prev
//     if (m_currentIndex == 0) {
//         const QMessageBox::StandardButton reply = QMessageBox::question(
//             nullptr,
//             tr("First match"),
//             tr("Go to bottom?"),
//             QMessageBox::Yes | QMessageBox::No,
//             QMessageBox::Yes);
//         if (reply == QMessageBox::Yes) {
//             m_currentIndex = m_searchList.length() - 1;
//         } else {
//             return;
//         }
//     } else {
//         m_currentIndex--;
//     }
//     searchHandle();
// }
//
// void EditorWidget::nextSearch() {
//     // go next
//     if (m_currentIndex == m_searchList.length() - 1) {
//         const QMessageBox::StandardButton reply = QMessageBox::question(
//             nullptr,
//             tr("Last match"),
//             tr("Go to top?"),
//             QMessageBox::Yes | QMessageBox::No,
//             QMessageBox::Yes);
//         if (reply == QMessageBox::Yes) {
//             m_currentIndex = 0;
//         } else {
//             return;
//         }
//     } else {
//         m_currentIndex++;
//     }
//     searchHandle();
// }
//
// void EditorWidget::cursorPositionSet(const int line, const int index) {
//     setCursorPosition(line, index);
//     setFocus();
// }
//
// void EditorWidget::cursorPositionGet(int *line, int *index) const {
//     getCursorPosition(line, index);
//     *line += 1;
// }
//
// void EditorWidget::selectionGet(int &indexFrom, int &indexTo, int &lineFrom, int &lineTo) const {
//     indexFrom = SendScintilla(SCI_GETSELECTIONSTART);
//     indexTo = SendScintilla(SCI_GETSELECTIONEND);
//     lineFrom = SendScintilla(SCI_LINEFROMPOSITION, indexFrom);
//     lineTo = SendScintilla(SCI_LINEFROMPOSITION, indexTo);
// }
//
/ TODO: This method is not undoable!
// void EditorWidget::textSet(const QString &text) {
//     setText(text);
//     breakpointLoad();
//     // regionLoad();
// }
//
// void EditorWidget::textInsert(const QString &text, const int line, const int index) {
//     const int pos = positionFromLineIndex(line, index);
//     beginUndoAction();
//     SendScintilla(SCI_INSERTTEXT, pos, text.toUtf8().constData()); // NOLINT
//     endUndoAction();
// }
//
// void EditorWidget::textReplace(const QString &text) {
//     // record index
//     const int index = m_currentIndex;
//     // replace current
//     SendScintilla(SCI_SETSEL, m_searchList[m_currentIndex][0], m_searchList[m_currentIndex][1]);
//     beginUndoAction();
//     SendScintilla(SCI_REPLACESEL, text.toUtf8().length(), text.toUtf8().constData()); // NOLINT
//     endUndoAction();
//     // refresh search list
//     textSearch(m_searchText, m_searchFlag);
//     m_currentIndex = index;
//     searchHandle();
// }
//
// void EditorWidget::textReplaceAll(const QString &text) {
//     // replace all
//     beginUndoAction();
//     for (int i = m_searchList.length() - 1; i >= 0; --i) {
//         SendScintilla(SCI_SETSEL, m_searchList[i][0], m_searchList[i][1]);
//         SendScintilla(SCI_REPLACESEL, text.toUtf8().length(), text.toUtf8().constData()); // NOLINT
//     }
//     endUndoAction();
//     // refresh search list
//     textSearch(m_searchText, m_searchFlag);
// }
//
// void EditorWidget::textReplace(const QString &text, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
//     beginUndoAction();
//     setSelection(lineFrom, indexFrom, lineTo, indexTo);
//     replaceSelectedText(text);
//     endUndoAction();
// }
//
// void EditorWidget::indicatorInsert(const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo, const int time) {
//     fillIndicatorRange(lineFrom, indexFrom, lineTo, indexTo, type);
//     if (time == -1) return;
//     QTimer::singleShot(time, [this, lineFrom, indexFrom, lineTo, indexTo, type] {
//         clearIndicatorRange(lineFrom, indexFrom, lineTo, indexTo, type);
//     });
// }
//
// void EditorWidget::indicatorRemove(const int type, const int lineFrom, const int indexFrom, const int lineTo, const int indexTo) {
//     if (lineFrom == -1) {
//         const int lastLine = lines() - 1;
//         const int lastIndex = lineLength(lastLine);
//         clearIndicatorRange(0, 0, lastLine, lastIndex, type);
//     } else {
//         clearIndicatorRange(lineFrom, indexFrom, lineTo, indexTo, type);
//     }
// }
//
// void EditorWidget::markerInsert(const int type, int line, const int time) {
//     markerAdd(line, type);
//     ensureLineVisible(line);
//     if (time == -1) return;
//     QTimer::singleShot(time, [this, line, type] {
//         markerDelete(line, type);
//     });
// }
//
// void EditorWidget::markerRemove(const int type, const int line) {
//     if (line == -1) {
//         markerDeleteAll(type);
//     } else {
//         markerDelete(line, type);
//     }
// }
//
// // EditorWidget protected
// void EditorWidget::focusOutEvent(QFocusEvent *event) {
//     // clear highlight
//     indicatorRemove(INDICATOR_HIGHLIGHT);
//     indicatorRemove(INDICATOR_READ);
//     indicatorRemove(INDICATOR_WRITE);
//     QsciScintilla::focusOutEvent(event);
// }
//
// void EditorWidget::keyPressEvent(QKeyEvent *event) {
//     m_dwellTimer->stop();
//     emit hideDwellWidget();
//     if (isReadOnly()) {
         // TODO: exclude search
//         emit requestPermission();
//         event->accept();
//         return;
//     }
//     if (event->modifiers() == Qt::ControlModifier) {
//         switch (event->key()) {
//             case Qt::Key_Slash: {
//                 commentHandle();
//                 event->accept();
//                 return;
//             }
//             break;
//             case Qt::Key_D: {
//                 duplicateHandle();
//                 event->accept();
//                 return;
//             }
//             break;
//             default: break;
//         }
//     }
//     if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up || event->key() == Qt::Key_Right || event->key() == Qt::Key_Down) {
//         QsciScintilla::keyPressEvent(event);
//         emit requestDocumentHighlight();
//         return;
//     }
//     QsciScintilla::keyPressEvent(event);
// }
//
// void EditorWidget::keyReleaseEvent(QKeyEvent *event) {
//     if (event->key() == Qt::Key_Control) {
//         indicatorRemove(INDICATOR_HYPERLINK);
//         viewport()->setCursor(Qt::IBeamCursor);
//     }
//     QsciScintilla::keyReleaseEvent(event);
// }
//
// void EditorWidget::mouseMoveEvent(QMouseEvent *event) {
//     // get current word
//     const QPoint globalPos = QCursor::pos();
//     const QPoint localPos = mapFromGlobal(globalPos);
//     if (!rect().contains(localPos)) {
//         m_currentWord.wordStart = -1;
//         m_currentWord.wordEnd = -1;
//         return;
//     };
//     const long charPos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, localPos.x(), localPos.y());
//     const long wordStart = SendScintilla(SCI_WORDSTARTPOSITION, charPos, true);
//     const long wordEnd = SendScintilla(SCI_WORDENDPOSITION, charPos, true);
//     if (wordStart != m_currentWord.wordStart || wordEnd != m_currentWord.wordEnd) {
//         m_currentWord.wordStart = wordStart;
//         m_currentWord.wordEnd = wordEnd;
//         m_dwellTimer->start();
//     }
//     if (event->modifiers() == Qt::ControlModifier) {
//         m_dwellTimer->stop();
//         indicatorRemove(INDICATOR_HYPERLINK);
//         viewport()->setCursor(Qt::IBeamCursor);
//         if (charPos != -1 && wordStart < wordEnd) {
//             const int LUA_TOKEN = SendScintilla(SCI_GETSTYLEAT, charPos);
//             if (LUA_TOKEN >= LUA_TOKEN_MACRO || LUA_TOKEN == 0) return;
//             const int lineFrom = SendScintilla(SCI_LINEFROMPOSITION, wordStart);
//             const int indexFrom = wordStart - SendScintilla(SCI_POSITIONFROMLINE, lineFrom);
//             const int lineTo = SendScintilla(SCI_LINEFROMPOSITION, wordEnd);
//             const int indexTo = wordEnd - SendScintilla(SCI_POSITIONFROMLINE, lineTo);
//             indicatorInsert(INDICATOR_HYPERLINK, lineFrom, indexFrom, lineTo, indexTo);
//             viewport()->setCursor(Qt::PointingHandCursor);
//         }
//         event->accept();
//         return;
//     }
//     QsciScintilla::mouseMoveEvent(event);
// }
//
// void EditorWidget::mousePressEvent(QMouseEvent *event) {
//     if (event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton) {
//         QsciScintilla::mousePressEvent(event);
//         emit requestDefinition();
//         emit requestReferences();
//         indicatorRemove(INDICATOR_HYPERLINK);
//         return;
//     }
//     if (event->button() == Qt::LeftButton) {
//         QsciScintilla::mousePressEvent(event);
//         emit requestDocumentHighlight();
//         return;
//     }
//     if (event->button() == Qt::RightButton) {
//         QsciScintilla::mousePressEvent(event);
//         m_dwellTimer->stop();
//         bool gotoMenu = true;
//         int line = 0;
//         int index = 0;
//         QString word{};
//         const QPoint globalPos = QCursor::pos();
//         const QPoint localPos = mapFromGlobal(globalPos);
//         // move cursor
//         long charPos = SendScintilla(SCI_POSITIONFROMPOINT, localPos.x(), localPos.y());
//         if (charPos != -1) {
//             if (!hasSelectedText() || charPos < SendScintilla(SCI_GETSELECTIONSTART) || charPos >= SendScintilla(SCI_GETSELECTIONEND)) {
//                 SendScintilla(SCI_GOTOPOS, charPos); // NOLINT
//             } else {
//                 word = selectedText();
//             }
//         }
//         // get word property
//         const long closePos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, localPos.x(), localPos.y());
//         const long wordStart = SendScintilla(SCI_WORDSTARTPOSITION, closePos, true);
//         const long wordEnd = SendScintilla(SCI_WORDENDPOSITION, closePos, true);
//         if (closePos != -1 && wordStart < wordEnd) {
//             // get style
//             const int LUA_TOKEN = SendScintilla(SCI_GETSTYLEAT, closePos);
//             // goto menu
//             if (LUA_TOKEN >= LUA_TOKEN_MACRO || LUA_TOKEN == 0) {
//                 gotoMenu = false;
//             }
//         } else {
//             gotoMenu = false;
//         }
//         const QVariantHash menuSession = {
//             {"gotoMenu", gotoMenu},
//             {"line", line},
//             {"index", index},
//             {"word", word}
//         };
//         emit showMenu(menuSession);
//         return;
//     }
//     QsciScintilla::mousePressEvent(event);
// }
//
// // EditorWidget private
// void EditorWidget::commentHandle() {
//     int startLine, startCharacter, endLine, endCharacter;
//     getSelection(&startLine, &startCharacter, &endLine, &endCharacter);
//     if (startLine == -1) {
//         getCursorPosition(&startLine, &startCharacter);
//         endLine = startLine;
//     }
//     beginUndoAction();
//     for (int line = startLine; line <= endLine; ++line) {
//         QString lineText = text(line);
//         if (lineText.startsWith("-- ")) {
//             setSelection(line, 0, line, 3);
//             removeSelectedText();
//         } else {
//             insertAt("-- ", line, 0);
//         }
//     }
//     endUndoAction();
// }
//
// void EditorWidget::duplicateHandle() {
//     int currentLine, currentCharacter;
//     getCursorPosition(&currentLine, &currentCharacter);
//     const QString lineText = text(currentLine);
//     beginUndoAction();
//     insertAt(lineText, currentLine + 1, 0);
//     setCursorPosition(currentLine + 1, currentCharacter);
//     endUndoAction();
// }
//
// void EditorWidget::dwellHandle() {
//     emit requestHover();
// }
//
// void EditorWidget::pairHandle(const int ascii) {
//     const auto input = QChar(ascii);
//     if (!m_autoPairHash.contains(input)) return;
//     insert(m_autoPairHash[input]);
// }
//
// void EditorWidget::searchHandle() {
//     // clear previous highlight
//     const int docLength = SendScintilla(SCI_GETLENGTH);
//     SendScintilla(SCI_SETINDICATORCURRENT, INDICATOR_SELECTION); // NOLINT
//     SendScintilla(SCI_INDICATORCLEARRANGE, 0, docLength); // NOLINT
//     if (m_searchList.empty()) {
//         m_currentIndex = 0;
//     } else {
//         if (m_currentIndex < 0) m_currentIndex = 0;
//         else if (m_currentIndex > m_searchList.length() - 1) m_currentIndex = m_searchList.length() - 1;
//         SendScintilla(SCI_GOTOPOS, m_searchList[m_currentIndex][0]); // NOLINT
//         const int line = SendScintilla(SCI_LINEFROMPOSITION, m_searchList[m_currentIndex][0]);
//         SendScintilla(SCI_ENSUREVISIBLE, line); // NOLINT
//         SendScintilla(SCI_SETINDICATORCURRENT, INDICATOR_SELECTION); // NOLINT
//         SendScintilla(SCI_INDICATORFILLRANGE, m_searchList[m_currentIndex][0], m_searchList[m_currentIndex][2]); // NOLINT
//     }
//     emit setStat(m_currentIndex, m_searchList.length());
//     SendScintilla(SCI_CLEARSELECTIONS); // NOLINT}
// }
//
// void EditorWidget::typeHandle() const {
//     m_typeTimer->start();
// }
