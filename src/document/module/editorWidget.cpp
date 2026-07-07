#include "document/module/editorWidget.h"

#include <QFileInfo>
#include <QFile>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <SciLexer.h>

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
      m_contentTimer(new QTimer(this)),
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
    connect(m_scintillaWidget, &ScintillaEdit::modified, this, [this](const Scintilla::ModificationFlags type) {
        if (static_cast<int>(type) & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_PERFORMED_UNDO | SC_PERFORMED_REDO)) m_contentTimer->start();
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
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &EditorWidget::contentChange);
}

void EditorWidget::propertySet(const QVariantHash &objects) {
    m_theme = objects["theme"].toJsonObject();
    m_propertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_gotoDialog = qvariant_cast<QObject *>(objects["documentModuleGotoDialog"]);
    m_searchWidget->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
    shortcutInit();
    miscInit();
    indicatorInit();
    marginInit();
    markerInit();
    styleInit();
    lexerInit();
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

void EditorWidget::documentGoto() {
    m_gotoDialog->setProperty("documentUrl", m_documentUrl);
    m_gotoDialog->setProperty("line", m_selection["line"]);
    m_gotoDialog->setProperty("character", m_selection["character"]);
    QMetaObject::invokeMethod(m_gotoDialog, "open");
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

void EditorWidget::contentChange() {
    emit changeContent();
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
            {"width", 16},
            {"mask", static_cast<int>(SC_MASK_FOLDERS)},
            {"sensitive", true},
            {"back", ScintillaWidget::colorGet(g_globalManager->backGet())}
        });
    m_scintillaWidget->marginDefine(
        2,
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

void EditorWidget::lexerInit() const {
    const QFileInfo documentInfo(m_documentUrl.toLocalFile());
    const auto suffix = documentInfo.suffix().toLower();
    const auto fileName = documentInfo.fileName().toLower();
    // json
    if (suffix == "json") {
        const auto jsonTheme = m_theme["json"].toObject();
        m_scintillaWidget->lexerSet("json");
        m_scintillaWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("lexer.json.escape.sequence"), reinterpret_cast<sptr_t>("1"));
        m_scintillaWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("lexer.json.allow.comments"), reinterpret_cast<sptr_t>("1"));
        m_scintillaWidget->send(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>("true false null"));
        m_scintillaWidget->styleDefine(
            SCE_JSON_NUMBER,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["number"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_STRING,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["string"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_STRINGEOL,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["stringEol"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_PROPERTYNAME,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["propertyName"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_ESCAPESEQUENCE,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["escapeSequence"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_LINECOMMENT,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["lineComment"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_BLOCKCOMMENT,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["blockComment"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_OPERATOR,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["operator"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_URI,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["uri"].toObject()["fore"].toString())},
                {"underline", jsonTheme["uri"].toObject()["underline"].toBool()}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_COMPACTIRI,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["compactIri"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_KEYWORD,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["keyword"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_LDKEYWORD,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["ldKeyword"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_JSON_ERROR,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(jsonTheme["error"].toObject()["fore"].toString())}
            });
    }
    // markdown
    else if (suffix == "md") {
        const auto markdownTheme = m_theme["markdown"].toObject();
        m_scintillaWidget->lexerSet("markdown");
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_STRONG1,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["strong1"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_STRONG2,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["strong2"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_EM1,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["em1"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_EM2,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["em2"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER1,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header1"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER2,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header2"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER3,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header3"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER4,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header4"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER5,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header5"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HEADER6,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["header6"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_ULIST_ITEM,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["ulistItem"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_OLIST_ITEM,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["olistItem"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_BLOCKQUOTE,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["blockquote"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_STRIKEOUT,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["strikeout"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_HRULE,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["hRule"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_LINK,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["link"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_CODE,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["code"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_CODE2,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["code2"].toObject()["fore"].toString())}
            });
        m_scintillaWidget->styleDefine(
            SCE_MARKDOWN_CODEBK,
            QVariantHash{
                {"fore", ScintillaWidget::colorGet(markdownTheme["codeBk"].toObject()["fore"].toString())}
            });
    }

    // abaqus
    else if (suffix == "aba" || suffix == "inp") {
        m_scintillaWidget->lexerSet("abaqus");
    }
    // ada
    else if (suffix == "adb" || suffix == "ads") {
        m_scintillaWidget->lexerSet("ada");
    }
    // apdl
    else if (suffix == "ans" || suffix == "mac") {
        m_scintillaWidget->lexerSet("apdl");
    }
    // asciidoc
    else if (suffix == "adoc" || suffix == "asciidoc") {
        m_scintillaWidget->lexerSet("asciidoc");
    }
    // asm
    else if (suffix == "asm" || suffix == "s" || suffix == "sx" || suffix == "a51") {
        m_scintillaWidget->lexerSet("asm");
    }
    // asn1
    else if (suffix == "asn" || suffix == "asn1") {
        m_scintillaWidget->lexerSet("asn1");
    }
    // asy
    else if (suffix == "asy") {
        m_scintillaWidget->lexerSet("asy");
    }
    // au3
    else if (suffix == "au3") {
        m_scintillaWidget->lexerSet("au3");
    }
    // bash
    else if (suffix == "sh" || suffix == "bash" || suffix == "zsh" || suffix == "ksh" || fileName == ".bashrc" || fileName == ".profile") {
        m_scintillaWidget->lexerSet("bash");
    }
    // batch
    else if (suffix == "bat" || suffix == "cmd" || suffix == "nt") {
        m_scintillaWidget->lexerSet("batch");
    }
    // bib
    else if (suffix == "bib") {
        m_scintillaWidget->lexerSet("bib");
    }
    // blitzbasic
    else if (suffix == "bb") {
        m_scintillaWidget->lexerSet("blitzbasic");
    }
    // caml
    else if (suffix == "ml" || suffix == "mli") {
        m_scintillaWidget->lexerSet("caml");
    }
    // cil
    else if (suffix == "il") {
        m_scintillaWidget->lexerSet("cil");
    }
    // cmake
    else if (suffix == "cmake" || fileName == "cmakelists.txt") {
        m_scintillaWidget->lexerSet("cmake");
    }
    // COBOL
    else if (suffix == "cob" || suffix == "cbl") {
        m_scintillaWidget->lexerSet("COBOL");
    }
    // coffeescript
    else if (suffix == "coffee" || suffix == "litcoffee") {
        m_scintillaWidget->lexerSet("coffeescript");
    }
    // conf
    else if (suffix == "cfg" || suffix == "conf") {
        m_scintillaWidget->lexerSet("conf");
    }
    // cpp
    else if (suffix == "c" || suffix == "cc" || suffix == "cpp" || suffix == "cxx" || suffix == "h" || suffix == "hh" || suffix == "hpp" || suffix == "hxx" || suffix == "inl" || suffix == "ino" || suffix == "js" || suffix == "ts") {
        m_scintillaWidget->lexerSet("cpp");
    }
    // csound
    else if (suffix == "csd" || suffix == "orc" || suffix == "sco") {
        m_scintillaWidget->lexerSet("csound");
    }
    // css
    else if (suffix == "css" || suffix == "scss" || suffix == "less") {
        m_scintillaWidget->lexerSet("css");
    }
    // d
    else if (suffix == "d") {
        m_scintillaWidget->lexerSet("d");
    }
    // dart
    else if (suffix == "dart") {
        m_scintillaWidget->lexerSet("dart");
    }
    // diff
    else if (suffix == "diff" || suffix == "patch") {
        m_scintillaWidget->lexerSet("diff");
    }
    // eiffel
    else if (suffix == "e") {
        m_scintillaWidget->lexerSet("eiffel");
    }
    // erlang
    else if (suffix == "erl" || suffix == "hrl") {
        m_scintillaWidget->lexerSet("erlang");
    }
    // fsharp
    else if (suffix == "fs" || suffix == "fsi" || suffix == "fsx") {
        m_scintillaWidget->lexerSet("fsharp");
    }
    // fortran
    else if (suffix == "f" || suffix == "for" || suffix == "f90" || suffix == "f95" || suffix == "f03" || suffix == "f08") {
        m_scintillaWidget->lexerSet("fortran");
    }
    // forth
    else if (suffix == "forth" || suffix == "fth") {
        m_scintillaWidget->lexerSet("forth");
    }
    // gdscript
    else if (suffix == "gd") {
        m_scintillaWidget->lexerSet("gdscript");
    }
    // haskell
    else if (suffix == "hs" || suffix == "lhs") {
        m_scintillaWidget->lexerSet("haskell");
    }
    // hypertext
    else if (suffix == "html" || suffix == "htm" || suffix == "xhtml" || suffix == "shtml" || suffix == "php") {
        m_scintillaWidget->lexerSet("hypertext");
    }
    // ihex
    else if (suffix == "hex") {
        m_scintillaWidget->lexerSet("ihex");
    }
    // inno
    else if (suffix == "iss") {
        m_scintillaWidget->lexerSet("inno");
    }
    // julia
    else if (suffix == "jl") {
        m_scintillaWidget->lexerSet("julia");
    }
    // latex
    else if (suffix == "tex" || suffix == "sty" || suffix == "ltx") {
        m_scintillaWidget->lexerSet("latex");
    }
    // lisp
    else if (suffix == "lisp" || suffix == "lsp" || suffix == "cl" || suffix == "el") {
        m_scintillaWidget->lexerSet("lisp");
    }
    // lua
    else if (suffix == "lua") {
        m_scintillaWidget->lexerSet("lua");
    }
    // makefile
    else if (suffix == "mak" || suffix == "mk" || fileName == "makefile") {
        m_scintillaWidget->lexerSet("makefile");
    }
    // matlab
    else if (suffix == "m") {
        m_scintillaWidget->lexerSet("matlab");
    }
    // maxima
    else if (suffix == "wxm") {
        m_scintillaWidget->lexerSet("maxima");
    }
    // metapost
    else if (suffix == "mp") {
        m_scintillaWidget->lexerSet("metapost");
    }
    // nim
    else if (suffix == "nim" || suffix == "nims") {
        m_scintillaWidget->lexerSet("nim");
    }
    // nix
    else if (suffix == "nix") {
        m_scintillaWidget->lexerSet("nix");
    }
    // nsis
    else if (suffix == "nsi" || suffix == "nsh") {
        m_scintillaWidget->lexerSet("nsis");
    }
    // null
    else if (suffix == "") {
        m_scintillaWidget->lexerSet("null");
    }
    // pascal
    else if (suffix == "pas" || suffix == "pp" || suffix == "inc") {
        m_scintillaWidget->lexerSet("pascal");
    }
    // perl
    else if (suffix == "pl" || suffix == "pm" || suffix == "pod") {
        m_scintillaWidget->lexerSet("perl");
    }
    // po
    else if (suffix == "po" || suffix == "pot") {
        m_scintillaWidget->lexerSet("po");
    }
    // powershell
    else if (suffix == "ps1" || suffix == "psm1" || suffix == "psd1") {
        m_scintillaWidget->lexerSet("powershell");
    }
    // props
    else if (suffix == "properties" || suffix == "ini" || suffix == "inf") {
        m_scintillaWidget->lexerSet("props");
    }
    // ps
    else if (suffix == "ps" || suffix == "eps") {
        m_scintillaWidget->lexerSet("ps");
    }
    // python
    else if (suffix == "py" || suffix == "pyw" || suffix == "pyi") {
        m_scintillaWidget->lexerSet("python");
    }
    // r
    else if (suffix == "r" || suffix == "rprofile" || suffix == "rmd") {
        m_scintillaWidget->lexerSet("r");
    }
    // registry
    else if (suffix == "reg") {
        m_scintillaWidget->lexerSet("registry");
    }
    // ruby
    else if (suffix == "rb" || suffix == "rbw" || fileName == "rakefile" || fileName == "gemfile") {
        m_scintillaWidget->lexerSet("ruby");
    }
    // rust
    else if (suffix == "rs") {
        m_scintillaWidget->lexerSet("rust");
    }
    // sas
    else if (suffix == "sas") {
        m_scintillaWidget->lexerSet("sas");
    }
    // SML
    else if (suffix == "sml" || suffix == "sig") {
        m_scintillaWidget->lexerSet("SML");
    }
    // sql
    else if (suffix == "sql") {
        m_scintillaWidget->lexerSet("sql");
    }
    // stata
    else if (suffix == "do" || suffix == "ado") {
        m_scintillaWidget->lexerSet("stata");
    }
    // tcl
    else if (suffix == "tcl" || suffix == "tk") {
        m_scintillaWidget->lexerSet("tcl");
    }
    // toml
    else if (suffix == "toml") {
        m_scintillaWidget->lexerSet("toml");
    }
    // troff
    else if (suffix == "man" || suffix == "me" || suffix == "ms" || suffix == "roff" || suffix == "tmac") {
        m_scintillaWidget->lexerSet("troff");
    }
    // vb
    else if (suffix == "vb" || suffix == "bas" || suffix == "frm" || suffix == "cls" || suffix == "ctl") {
        m_scintillaWidget->lexerSet("vb");
    }
    // vbscript
    else if (suffix == "vbs") {
        m_scintillaWidget->lexerSet("vbscript");
    }
    // verilog
    else if (suffix == "v" || suffix == "vh" || suffix == "sv" || suffix == "svh") {
        m_scintillaWidget->lexerSet("verilog");
    }
    // vhdl
    else if (suffix == "vhd" || suffix == "vhdl") {
        m_scintillaWidget->lexerSet("vhdl");
    }
    // xml
    else if (suffix == "xml" || suffix == "xsd" || suffix == "xsl" || suffix == "xslt" || suffix == "svg" || suffix == "ui" || suffix == "qrc") {
        m_scintillaWidget->lexerSet("xml");
    }
    // yaml
    else if (suffix == "yaml" || suffix == "yml") {
        m_scintillaWidget->lexerSet("yaml");
    }
    // zig
    else if (suffix == "zig") {
        m_scintillaWidget->lexerSet("zig");
    }
    // automatic
    else {
        const auto lexerName = suffix.toUtf8();
        m_scintillaWidget->lexerSet(lexerName.constData());
    }
    m_scintillaWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1"));
    m_scintillaWidget->send(SCI_COLOURISE, 0, -1); // NOLINT
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

void EditorWidget::permissionSet() const {
    m_propertyDialog->setProperty("documentUrl", m_documentUrl);
    QMetaObject::invokeMethod(m_propertyDialog, "permission");
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
