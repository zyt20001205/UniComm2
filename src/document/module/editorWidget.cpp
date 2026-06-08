#include "document/module/editorWidget.h"

#include <QFileInfo>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "core/globalManager.h"
#include "document/module/scintillaWidget.h"
#include "document/module/searchWidget.h"

// public
EditorWidget::EditorWidget(const QJsonObject &documentConfig, const QUrl &documentUrl, QWidget *parent)
    : QWidget(parent),
      m_documentUrl(documentUrl),
      m_scintillaWidget(new ScintillaWidget(this)),
      m_config(documentConfig),
      m_searchWidget(new SearchWidget(this)),
      m_selectionTimer(new QTimer(this)),
      m_pair{
          {'"', '"'},
          {'\'', '\''},
          {'(', ')'},
          {'[', ']'},
          {'{', '}'}
      } {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_searchWidget);
    layout->addWidget(m_scintillaWidget);

    documentOpen();
    connect(m_scintillaWidget, &ScintillaEdit::modifyAttemptReadOnly, this, &EditorWidget::permissionSet);
    connect(m_scintillaWidget, &ScintillaEdit::savePointChanged, this, &EditorWidget::changeSavepoint);
    connect(m_scintillaWidget, &ScintillaEdit::updateUi, this, [this](const Scintilla::Update updated) {
        if (static_cast<int>(updated) & static_cast<int>(Scintilla::Update::Selection | Scintilla::Update::Content)) m_selectionTimer->start();
    });
    connect(m_scintillaWidget, &ScintillaEdit::focusChanged, this, [this](const bool focused) {
        if (focused) m_selectionTimer->start();
    });
    connect(m_searchWidget, &SearchWidget::setSearchFlags, m_scintillaWidget, &ScintillaWidget::searchFlagsSet);
    connect(m_searchWidget, &SearchWidget::requestSearch, this, &EditorWidget::searchRequest);
    connect(m_searchWidget, &SearchWidget::prevSearch, this, &EditorWidget::searchPrev);
    connect(m_searchWidget, &SearchWidget::nextSearch, this, &EditorWidget::searchNext);
    connect(m_searchWidget, &SearchWidget::replaceText, this, &EditorWidget::textReplace);
    connect(m_searchWidget, &SearchWidget::replaceAll, this, &EditorWidget::allReplace);
    m_scintillaWidget->installEventFilter(this);
    // 100ms debounce for selection change
    m_selectionTimer->setSingleShot(true);
    m_selectionTimer->setInterval(100);
    connect(m_selectionTimer, &QTimer::timeout, this, &EditorWidget::selectionChange);
}

void EditorWidget::propertySet(const QVariantHash &objects) {
    m_propertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_gotoDialog = qvariant_cast<QObject *>(objects["documentModuleGotoDialog"]);
    m_searchWidget->propertySet(QVariantHash{
        {"global", objects["global"]},
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
    shortcutInit();
    miscInit();
    indicatorInit();
    marginInit();
    markerInit();
    styleInit();
}

void EditorWidget::documentSave() {
    if (!m_scintillaWidget->modifyGet()) return;
    // savepoint set
    m_scintillaWidget->savepointSet();
    // text get
    const auto documentPath = m_documentUrl.toLocalFile();
    auto documentFile = QFile(documentPath);
    if (!documentFile.open(QIODevice::WriteOnly)) return;
    auto documentTextStream = QTextStream(&documentFile);
    documentTextStream << m_scintillaWidget->textGet();
    documentFile.close();
    // logging
    emit appendLog(LogLevel::Info, "document saved", QString("<a href='%1'>%2</a>").arg(m_documentUrl.toString(), m_documentUrl.toString()));
}

bool EditorWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_scintillaWidget && event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
            case Qt::Key_QuoteDbl:
                return symbolPair('"');
            case Qt::Key_Apostrophe:
                return symbolPair('\'');
            case Qt::Key_ParenLeft:
                return symbolPair('(');
            case Qt::Key_BracketLeft:
                return symbolPair('[');
            case Qt::Key_BraceLeft:
                return symbolPair('{');
            case Qt::Key_Escape:
                if (m_searchWidget->isVisible()) m_searchWidget->hide();
                return false;
            case Qt::Key_Backspace:
                if (m_scintillaWidget->selectionEmpty() && !m_scintillaWidget->atLineEnd()) {
                    return symbolPair('\b');
                }
                return false;
            default: return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

// protected
void EditorWidget::shortcutInit() {
    auto shortcutDuplicate = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this); // NOLINT
    shortcutDuplicate->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutDuplicate, &QShortcut::activated, m_scintillaWidget, &ScintillaWidget::lineDuplicate);
    auto shortcutGoto = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_G), this); // NOLINT
    shortcutGoto->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutGoto, &QShortcut::activated, this, &EditorWidget::documentGoto);
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutSearch, &QShortcut::activated, this, &EditorWidget::searchShow);
    auto shortcutReplace = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this); // NOLINT
    shortcutReplace->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcutReplace, &QShortcut::activated, this, &EditorWidget::replaceShow);
}

void EditorWidget::selectionChange() {
    m_selection = m_scintillaWidget->selectionGet();
    emit changeSelection(m_selection);
}

bool EditorWidget::symbolPair(const QChar ch) {
    // handle backspace
    if (ch == '\b') {
        const auto line = m_selection["line"];
        const auto character = m_selection["character"];
        if (character > 0) {
            const auto prevChar = m_scintillaWidget->textGet(line, character - 1, line, character)[0];
            const auto nextChar = m_scintillaWidget->textGet(line, character, line, character + 1);
            if (m_pair.contains(prevChar) && !nextChar.isEmpty() && m_pair[prevChar] == nextChar) {
                m_scintillaWidget->textSet("", line, character - 1, line, character + 1);
                return true;
            }
        }
        return false;
    }
    // handle pair
    if (m_scintillaWidget->selectionEmpty()) {
        m_scintillaWidget->textSetSelected(QString(ch) + m_pair[ch]);
        const auto position = m_scintillaWidget->positionGet();
        m_scintillaWidget->positionSet(position - 1);
    }
    // handle surround
    else {
        m_scintillaWidget->textSetSelected(ch + m_scintillaWidget->textGetSelected() + m_pair[ch]);
    }
    return true;
}

void EditorWidget::miscInit() const {
    // annotation
    m_scintillaWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
    m_scintillaWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
    // folding
    m_scintillaWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1")); // NOLINT
    m_scintillaWidget->send(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE); // NOLINT
    m_scintillaWidget->send(SCI_SETFOLDMARGINCOLOUR, true, ScintillaWidget::colorGet(g_globalManager->backGet())); // NOLINT
    m_scintillaWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, ScintillaWidget::colorGet(g_globalManager->backGet())); // NOLINT
    m_scintillaWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
    m_scintillaWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
    // scrollbar
    m_scintillaWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
    m_scintillaWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT
    // selection
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, ScintillaWidget::colorGet(g_globalManager->brandBackGet(), 128)); // NOLINT
    m_scintillaWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET, ScintillaWidget::colorGet(g_globalManager->foreGet(), 255)); // NOLINT
    m_scintillaWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, ScintillaWidget::colorGet(g_globalManager->backSelectedGet(), 128)); // NOLINT
    m_scintillaWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
    // tab
    m_scintillaWidget->send(SCI_SETUSETABS, false); // NOLINT
    m_scintillaWidget->send(SCI_SETINDENT, 4); // NOLINT
    m_scintillaWidget->send(SCI_SETTABINDENTS, true); // NOLINT
    m_scintillaWidget->send(SCI_SETBACKSPACEUNINDENTS, true); // NOLINT
    m_scintillaWidget->send(SCI_SETINDENTATIONGUIDES, SC_IV_REAL); // NOLINT
}

void EditorWidget::indicatorInit() const {
    m_scintillaWidget->indicatorDefine(
        ScintillaIndicator::Result,
        QVariantHash{
            {"style", INDIC_STRAIGHTBOX},
            {"fore", ScintillaWidget::colorGet(g_globalManager->brandBackGet())},
            {"alpha", 128},
            {"outlineAlpha", 128},
            {"setUnder", true}
        });
    m_scintillaWidget->indicatorDefine(
        ScintillaIndicator::Current,
        QVariantHash{
            {"style", INDIC_STRAIGHTBOX},
            {"fore", ScintillaWidget::colorGet(g_globalManager->foreGet())},
            {"strokeWidth", 200},
            {"alpha", 0},
            {"outlineAlpha", 255},
            {"setUnder", false}
        });
}

void EditorWidget::marginInit() const {
    m_scintillaWidget->marginDefine(
        0,
        QVariantHash{
            {"type", SC_MARGIN_NUMBER},
            {"width", 32}
        });
    m_scintillaWidget->marginDefine(
        1,
        QVariantHash{
            {"type", SC_MARGIN_SYMBOL},
            {"width", 4},
            {"mask", SC_MASK_HISTORY},
        });
}

void EditorWidget::markerInit() const {
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDEREND,
        QVariantHash{
            {"symbol", SC_MARK_BOXPLUSCONNECTED},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDEROPENMID,
        QVariantHash{
            {"symbol", SC_MARK_BOXMINUSCONNECTED},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDERMIDTAIL,
        QVariantHash{
            {"symbol", SC_MARK_TCORNER},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDERTAIL,
        QVariantHash{
            {"symbol", SC_MARK_LCORNER},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDERSUB,
        QVariantHash{
            {"symbol", SC_MARK_VLINE},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDER,
        QVariantHash{
            {"symbol", SC_MARK_BOXPLUS},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
    m_scintillaWidget->markerDefine(
        SC_MARKNUM_FOLDEROPEN,
        QVariantHash{
            {"symbol", SC_MARK_BOXMINUS},
            {"fore", ScintillaWidget::colorGet(g_globalManager->backGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->strokeGet())}
        });
}

void EditorWidget::styleInit() const {
    m_scintillaWidget->styleDefine(
        CustomStyle::Default,
        QVariantHash{
            {"font", m_config["fontFamily"].toString()},
            {"size", m_config["fontSize"].toInt()},
            {"fore", ScintillaWidget::colorGet(g_globalManager->foreGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
    m_scintillaWidget->styleClearAll();
    m_scintillaWidget->styleDefine(
        CustomStyle::LineNumber,
        QVariantHash{
            {"fore", ScintillaWidget::colorGet(g_globalManager->strokeGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
    m_scintillaWidget->styleDefine(
        CustomStyle::FoldDisplayText,
        QVariantHash{
            {"fore", ScintillaWidget::colorGet(g_globalManager->foreGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->backSelectedGet())}
        });
    m_scintillaWidget->styleDefine(
        CustomStyle::Annotation,
        QVariantHash{
            {"fore", ScintillaWidget::colorGet(g_globalManager->strokeGet())},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
}

void EditorWidget::searchShow() const {
    m_searchWidget->searchShow(m_scintillaWidget->textGetSelected());
    m_searchWidget->show();
}

void EditorWidget::replaceShow() const {
    m_searchWidget->replaceShow(m_scintillaWidget->textGetSelected());
    m_searchWidget->show();
}

// private
void EditorWidget::documentOpen() const {
    // text get
    const auto documentPath = m_documentUrl.toLocalFile();
    auto documentFile = QFile(documentPath);
    if (!documentFile.open(QIODevice::ReadOnly)) return;
    auto documentTextStream = QTextStream(&documentFile);
    const auto documentText = documentTextStream.readAll();
    documentFile.close();
    m_scintillaWidget->textSet(documentText);
    // permission set
    const auto documentInfo = QFileInfo(documentPath);
    m_scintillaWidget->readonlySet(!documentInfo.isWritable());
    // history set
    m_scintillaWidget->send(SCI_EMPTYUNDOBUFFER); // NOLINT
    m_scintillaWidget->send(SCI_SETCHANGEHISTORY,SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS); // NOLINT
}

void EditorWidget::documentGoto() {
    m_gotoDialog->setProperty("documentUrl", m_documentUrl);
    m_gotoDialog->setProperty("line", m_selection["line"]);
    m_gotoDialog->setProperty("character", m_selection["character"]);
    QMetaObject::invokeMethod(m_gotoDialog, "open");
}

void EditorWidget::permissionSet() const {
    m_propertyDialog->setProperty("fileUrl", m_documentUrl);
    QMetaObject::invokeMethod(m_propertyDialog, "open");
}

// private: search
void EditorWidget::searchRequest(const QString &text) {
    searchClear();
    m_search["text"] = text;
    if (!text.isEmpty()) {
        int current = 0;
        int total = 0;
        QVariantList startList{};
        QVariantList endList{};

        m_scintillaWidget->targetSetWhole();
        while (true) {
            if (m_scintillaWidget->targetSearch(text) == -1) break;
            const auto start = m_scintillaWidget->targetGetStart();
            const auto end = m_scintillaWidget->targetGetEnd();
            startList.append(start);
            endList.append(end);
            const auto startIndex = m_scintillaWidget->indexGet(start);
            const auto endIndex = m_scintillaWidget->indexGet(end);
            m_scintillaWidget->indicatorFill(
                ScintillaIndicator::Result,
                startIndex["line"],
                startIndex["character"],
                endIndex["line"],
                endIndex["character"]
            );
            if (m_selection["startPosition"] > start) current++;
            m_scintillaWidget->targetSetStart(end);
            m_scintillaWidget->targetSetEnd(m_scintillaWidget->lengthGet());
        }
        total = static_cast<int>(startList.size());
        if (current == total) current--;

        m_search["current"] = current;
        m_search["total"] = total;
        m_search["start"] = startList;
        m_search["end"] = endList;
    }
    searchResponse();
}

void EditorWidget::searchResponse() {
    if (m_search["total"].toInt() == 0) {
        m_searchWidget->searchEnable(false);
        m_searchWidget->replaceEnable(false);
        m_searchWidget->searchResponse("0/0");
        return;
    }
    m_searchWidget->searchEnable(true);
    m_searchWidget->replaceEnable(true);
    const auto total = m_search["total"].toInt();
    const auto current = m_search["current"].toInt();
    m_searchWidget->searchResponse(QString("%1/%2").arg(QString::number(current + 1), QString::number(total)));
    const auto startList = m_search["start"].toList();
    const auto endList = m_search["end"].toList();
    const auto startIndex = m_scintillaWidget->indexGet(startList[current].toInt());
    const auto endIndex = m_scintillaWidget->indexGet(endList[current].toInt());
    m_scintillaWidget->indexSet(
        startIndex["line"],
        startIndex["character"]
    );
    m_scintillaWidget->indicatorFill(
        ScintillaIndicator::Current,
        startIndex["line"],
        startIndex["character"],
        endIndex["line"],
        endIndex["character"]
    );
}

void EditorWidget::searchPrev() {
    m_scintillaWidget->indicatorClear(ScintillaIndicator::Current);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != 0) {
        m_search["current"] = current - 1;
    } else {
        m_search["current"] = total - 1;
    }
    searchResponse();
}

void EditorWidget::searchNext() {
    m_scintillaWidget->indicatorClear(ScintillaIndicator::Current);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != total - 1) {
        m_search["current"] = current + 1;
    } else {
        m_search["current"] = 0;
    }
    searchResponse();
}

void EditorWidget::searchClear() {
    m_search.clear();
    m_scintillaWidget->indicatorClear(ScintillaIndicator::Result);
    m_scintillaWidget->indicatorClear(ScintillaIndicator::Current);
}

void EditorWidget::textReplace(const QString &text) {
    const auto current = m_search["current"].toInt();
    const auto startList = m_search["start"].toList();
    const auto endList = m_search["end"].toList();
    const auto startIndex = m_scintillaWidget->indexGet(startList[current].toInt());
    const auto endIndex = m_scintillaWidget->indexGet(endList[current].toInt());
    m_scintillaWidget->indexSet(
        startIndex["line"],
        startIndex["character"]
    );
    m_scintillaWidget->textSet(
        text,
        startIndex["line"],
        startIndex["character"],
        endIndex["line"],
        endIndex["character"]
    );
    m_searchWidget->searchRequest();
}

void EditorWidget::allReplace(const QString &text) {
    m_scintillaWidget->undoBegin();
    for (int index = m_search["total"].toInt() - 1; index >= 0; --index) {
        const auto startList = m_search["start"].toList();
        const auto endList = m_search["end"].toList();
        const auto startIndex = m_scintillaWidget->indexGet(startList[index].toInt());
        const auto endIndex = m_scintillaWidget->indexGet(endList[index].toInt());
        m_scintillaWidget->textSet(
            text,
            startIndex["line"],
            startIndex["character"],
            endIndex["line"],
            endIndex["character"]
        );
    }
    m_scintillaWidget->undoEnd();
    m_searchWidget->searchRequest();
}
