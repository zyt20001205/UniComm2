#include "document/page/luaPage.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QLineEdit>
#include <QProcess>
#include <QShortcut>
#include <QTemporaryFile>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "analysis/symbolWidget.h"
#include "core/globalManager.h"
#include "document/module/scintillaWidget.h"
#include "document/module/searchWidget.h"
#include "util/cmarkUtils.h"

// public
LuaPage::LuaPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_editorWidget(new ScintillaWidget(this)),
      m_searchWidget(new SearchWidget(this)),
      m_symbolWidget(new SymbolWidget(this)),
      m_assemblyWidget(new ScintillaWidget(this)),
      m_selectionTimer(new QTimer(this)),
      m_contentTimer(new QTimer(this)),
      m_dwellTimer(new QTimer(this)),
      m_completionSet{'.', ':', '\'', '"', '[', '#', '*', '@', '|', '=', '-', '{', '+', '?'},
      m_signatureHelpSet{'(', ','},
      m_onTypeFormattingSet{'\n'},
      m_pairHash{{'"', '"'}, {'\'', '\''}, {'(', ')'}, {'[', ']'}, {'{', '}'}} {
    setTitle(documentUrl.fileName());
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    connect(shortcutSearch, &QShortcut::activated, this, &LuaPage::searchToggle);
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutReplace = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this); // NOLINT
    connect(shortcutReplace, &QShortcut::activated, this, &LuaPage::replaceToggle);
    shortcutReplace->setContext(Qt::WidgetWithChildrenShortcut);

    auto shortcutLineDuplicate = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this); // NOLINT
    connect(shortcutLineDuplicate, &QShortcut::activated, m_editorWidget, &ScintillaWidget::lineDuplicate);
    shortcutLineDuplicate->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutComment = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash), this); // NOLINT
    connect(shortcutComment, &QShortcut::activated, this, &LuaPage::commentToggle);
    shortcutComment->setContext(Qt::WidgetWithChildrenShortcut);

    // 100ms debounce for selection change
    m_selectionTimer->setSingleShot(true);
    m_selectionTimer->setInterval(100);
    connect(m_selectionTimer, &QTimer::timeout, this, &LuaPage::selectionChange);
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &LuaPage::contentChange);
    // 1000ms debounce for dwell change
    m_dwellTimer->setSingleShot(true);
    m_dwellTimer->setInterval(1000);
    connect(m_dwellTimer, &QTimer::timeout, this, &LuaPage::dwellChange);

    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *codingwidget = new QWidget(this); // NOLINT
    auto *codingLayout = new QHBoxLayout(codingwidget); // NOLINT
    codingLayout->setContentsMargins(0, 0, 0, 0);
    codingLayout->setSpacing(0);
    codingLayout->addWidget(m_editorWidget);
    codingLayout->addWidget(m_assemblyWidget);
    // editor init
    {
        // misc
        {
            m_editorWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
            m_editorWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT

            m_editorWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1")); // NOLINT
            m_editorWidget->send(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS); // NOLINT
            m_editorWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS); // NOLINT
            for (int i = SC_MARKNUM_FOLDEREND; i <= SC_MARKNUM_FOLDEROPEN; ++i) {
                m_editorWidget->send(SCI_MARKERSETFORE, i, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
                m_editorWidget->send(SCI_MARKERSETBACK, i, ScintillaWidget::colorGet(g_global->strokeGet())); // NOLINT
            }
            m_editorWidget->send(SCI_SETFOLDMARGINCOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
            m_editorWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
            m_editorWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
            m_editorWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
            // TODO: hotspot is not working for STYLE_FOLDDISPLAYTEXT
            // m_editorWidget->send(SCI_STYLESETHOTSPOT, STYLE_FOLDDISPLAYTEXT, true); // NOLINT

            m_editorWidget->send(SCI_SETUSETABS, false); // NOLINT
            m_editorWidget->send(SCI_SETINDENT, 4); // NOLINT
            m_editorWidget->send(SCI_SETTABINDENTS, true); // NOLINT
            m_editorWidget->send(SCI_SETBACKSPACEUNINDENTS, true); // NOLINT
            m_editorWidget->send(SCI_SETINDENTATIONGUIDES, SC_IV_REAL); // NOLINT

            m_editorWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
            m_editorWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT

            m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, ScintillaWidget::colorGet(g_global->brandBackGet(), 128)); // NOLINT
            m_editorWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET, ScintillaWidget::colorGet(g_global->foreGet(), 255)); // NOLINT
            m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, ScintillaWidget::colorGet(g_global->backSelectedGet(), 128)); // NOLINT
            m_editorWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            // load
            const QUrl &url(documentUrl);
            const QString documentPath = url.toLocalFile();
            QFile file(documentPath);
            if (!file.open(QIODevice::ReadOnly)) return;
            QTextStream in(&file);
            const QString text = in.readAll();
            file.close();
            m_editorWidget->textSet(text);
            // permission
            const QFileInfo documentInfo(documentPath);
            m_editorWidget->readonlySet(!documentInfo.isWritable());
            // history
            m_editorWidget->send(SCI_EMPTYUNDOBUFFER); // NOLINT
            m_editorWidget->send(SCI_SETCHANGEHISTORY,SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS); // NOLINT
        }
        // font
        m_editorWidget->fontSet(QFont(documentConfig["fontFamily"].toString(), documentConfig["fontSize"].toInt()));
        // indicator
        {
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Typo,
                QJsonObject{
                    {"style", 1},
                    {"fore", ScintillaWidget::colorGet(g_global->successFore3Get())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Hint,
                QJsonObject{
                    {"style", 1},
                    {"fore", ScintillaWidget::colorGet(g_global->brandBackGet())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Info,
                QJsonObject{
                    {"style", 1},
                    {"fore", ScintillaWidget::colorGet(g_global->warningFore3Get())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Warning,
                QJsonObject{
                    {"style", 1},
                    {"fore", ScintillaWidget::colorGet(g_global->warningFore3Get())},
                    {"strokeWidth", 200},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Error,
                QJsonObject{
                    {"style", 1},
                    {"fore", ScintillaWidget::colorGet(g_global->dangerFore3Get())},
                    {"strokeWidth", 200},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Password,
                QJsonObject{
                    {"style", 8},
                    {"fore", ScintillaWidget::colorGet(g_global->backGet())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false},
                    {"hoverStyle", 5}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Highlight,
                QJsonObject{
                    {"style", 8},
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Read,
                QJsonObject{
                    {"style", 18},
                    {"fore", ScintillaWidget::colorGet(g_global->successFore3Get())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Write,
                QJsonObject{
                    {"style", 22},
                    {"fore", ScintillaWidget::colorGet(g_global->warningFore3Get())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Search,
                QJsonObject{
                    {"style", 8},
                    {"fore", ScintillaWidget::colorGet(g_global->brandBackGet())},
                    {"alpha", 128},
                    {"outlineAlpha", 128},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Selection,
                QJsonObject{
                    {"style", 6},
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"strokeWidth", 200},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            m_editorWidget->indicatorDefine(
                ScintillaIndicator::Hyperlink,
                QJsonObject{
                    {"style", 17},
                    {"fore", ScintillaWidget::colorGet(g_global->brandLinkGet())},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
        }
        // margin
        {
            m_editorWidget->marginDefine(
                0,
                QJsonObject{
                    {"type", SC_MARGIN_NUMBER},
                    {"width", 32},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->marginDefine(
                1,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(~SC_MASK_FOLDERS & ~SC_MASK_HISTORY)},
                    {"sensitive", true},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->marginDefine(
                2,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(SC_MASK_FOLDERS)},
                    {"sensitive", true},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->marginDefine(
                3,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 4},
                    {"mask", SC_MASK_HISTORY},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
        }
        // marker
        {
            m_editorWidget->markerDefine(
                ScintillaMarker::Region,
                QJsonObject{
                    {"symbol", 2},
                    {"fore", ScintillaWidget::colorGet(g_global->successFore2Get())},
                    {"back", ScintillaWidget::colorGet(g_global->successBack2Get())}
                });
            m_editorWidget->markerDefine(
                ScintillaMarker::BreakpointEnabled,
                QJsonObject{
                    {"symbol", 0},
                    {"fore", ScintillaWidget::colorGet(g_global->dangerFore2Get())},
                    {"back", ScintillaWidget::colorGet(g_global->dangerBack2Get())}
                });
            m_editorWidget->markerDefine(
                ScintillaMarker::BreakpointDisabled,
                QJsonObject{
                    {"symbol", 0},
                    {"fore", ScintillaWidget::colorGet(g_global->dangerFore2Get())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->markerDefine(
                ScintillaMarker::Navigation,
                QJsonObject{
                    {"symbol", 24},
                    {"fore", ScintillaWidget::colorGet(g_global->foreGet())}
                });
            m_editorWidget->markerDefine(
                ScintillaMarker::Debug,
                QJsonObject{
                    {"symbol", 2},
                    {"fore", ScintillaWidget::colorGet(g_global->warningFore2Get())},
                    {"back", ScintillaWidget::colorGet(g_global->warningFore2Get())}
                });
            m_editorWidget->markerDefine(
                ScintillaMarker::Hint,
                QJsonObject{
                    {"symbol", 22},
                    {"back", ScintillaWidget::colorGet(g_global->strokeGet())}
                });
        }
        // style
        {
            m_editorWidget->styleDefine(
                CustomStyle::Default,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->foreGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->styleClearAll();
            m_editorWidget->styleDefine(
                CustomStyle::LineNumber,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_editorWidget->styleDefine(
                CustomStyle::FoldDisplayText,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->foreGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backSelectedGet())}
                });
            m_editorWidget->styleDefine(
                CustomStyle::Annotation,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
        }
        // signals
        connect(m_editorWidget, &ScintillaEdit::modifyAttemptReadOnly, this, &LuaPage::permissionSet);
        connect(m_editorWidget, &ScintillaEdit::notify, this, [this](const Scintilla::NotificationData *pscn) {
            switch (pscn->nmhdr.code) {
                case Scintilla::Notification::MarginClick: {
                    marginClick(pscn->position, Qt::LeftButton, pscn->modifiers, pscn->margin);
                }
                break;
                case Scintilla::Notification::MarginRightClick: {
                    marginClick(pscn->position, Qt::RightButton, pscn->modifiers, pscn->margin);
                }
                break;
                default: break;
            }
        });
        connect(m_editorWidget, &ScintillaEdit::charAdded, this, &LuaPage::charAdd);
        connect(m_editorWidget, &ScintillaEdit::updateUi, this, [this](const Scintilla::Update updated) {
            if (updated == Scintilla::Update::Selection) m_selectionTimer->start();
        });
        connect(m_editorWidget, &ScintillaEdit::savePointChanged, this, &LuaPage::savepointChange);
        connect(m_editorWidget, &ScintillaEdit::modified, this, [this](const Scintilla::ModificationFlags type) {
            if (static_cast<int>(type) & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_PERFORMED_UNDO | SC_PERFORMED_REDO)) m_contentTimer->start();
        });
        connect(m_editorWidget, &ScintillaEdit::hotSpotClick, this, [](Scintilla::Position position, Scintilla::KeyMod modifiers) {
            qDebug() << "hotspot triggered!";
        });
        m_editorWidget->installEventFilter(this);
        m_editorWidget->viewport()->installEventFilter(this);
    }
    themeLoad(g_mainConfig["theme"].toInt());
    // assembly init
    {
        m_assemblyWidget->hide();
        // misc
        {
            m_assemblyWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
            m_assemblyWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT

            m_assemblyWidget->send(SCI_SETPROPERTY, reinterpret_cast<sptr_t>("fold"), reinterpret_cast<sptr_t>("1")); // NOLINT
            m_assemblyWidget->send(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS); // NOLINT
            m_assemblyWidget->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS); // NOLINT
            for (int i = SC_MARKNUM_FOLDEREND; i <= SC_MARKNUM_FOLDEROPEN; ++i) {
                m_assemblyWidget->send(SCI_MARKERSETFORE, i, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
                m_assemblyWidget->send(SCI_MARKERSETBACK, i, ScintillaWidget::colorGet(g_global->strokeGet())); // NOLINT
            }
            m_assemblyWidget->send(SCI_SETFOLDMARGINCOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
            m_assemblyWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, ScintillaWidget::colorGet(g_global->backGet())); // NOLINT
            m_assemblyWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
            m_assemblyWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
            // TODO: hotspot is not working for STYLE_FOLDDISPLAYTEXT
            // m_assemblyWidget->send(SCI_STYLESETHOTSPOT, STYLE_FOLDDISPLAYTEXT, true); // NOLINT

            m_assemblyWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
            m_assemblyWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT

            m_assemblyWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, ScintillaWidget::colorGet(g_global->brandBackGet(), 128)); // NOLINT
            m_assemblyWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            m_assemblyWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET, ScintillaWidget::colorGet(g_global->foreGet(), 255)); // NOLINT
            m_assemblyWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, ScintillaWidget::colorGet(g_global->backSelectedGet(), 128)); // NOLINT
            m_assemblyWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
        }
        // font
        m_assemblyWidget->fontSet(QFont(documentConfig["fontFamily"].toString(), documentConfig["fontSize"].toInt()));
        // margin
        {
            m_assemblyWidget->marginDefine(
                0,
                QJsonObject{
                    {"type", SC_MARGIN_TEXT},
                    {"width", 32},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_assemblyWidget->marginDefine(
                1,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(~SC_MASK_FOLDERS)},
                    {"sensitive", true},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_assemblyWidget->marginDefine(
                2,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(SC_MASK_FOLDERS)},
                    {"sensitive", true},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
        }
        // marker
        {
            m_assemblyWidget->markerDefine(
                ScintillaMarker::Hint,
                QJsonObject{
                    {"symbol", 22},
                    {"back", ScintillaWidget::colorGet(g_global->strokeGet())}
                });
        }
        // style
        {
            m_assemblyWidget->styleDefine(
                CustomStyle::Default,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->foreGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_assemblyWidget->styleClearAll();
            m_assemblyWidget->styleDefine(
                CustomStyle::LineNumber,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
            m_assemblyWidget->styleDefine(
                CustomStyle::FoldDisplayText,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->foreGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backSelectedGet())}
                });
            m_assemblyWidget->styleDefine(
                CustomStyle::Annotation,
                QJsonObject{
                    {"fore", ScintillaWidget::colorGet(g_global->strokeGet())},
                    {"back", ScintillaWidget::colorGet(g_global->backGet())}
                });
        }
    }

    layout->addWidget(m_searchWidget);
    layout->addWidget(codingwidget);
    layout->addWidget(m_symbolWidget);
    connect(m_searchWidget, &SearchWidget::setSearchFlags, m_editorWidget, &ScintillaWidget::searchFlagsSet);
    connect(m_searchWidget, &SearchWidget::requestSearch, this, &LuaPage::searchRequest);
    connect(m_searchWidget, &SearchWidget::prevSearch, this, &LuaPage::searchPrev);
    connect(m_searchWidget, &SearchWidget::nextSearch, this, &LuaPage::searchNext);
    connect(m_searchWidget, &SearchWidget::replaceText, this, &LuaPage::textReplace);
    connect(m_searchWidget, &SearchWidget::replaceAll, this, &LuaPage::allReplace);
    connect(m_symbolWidget, &SymbolWidget::appendLog, this, &LuaPage::appendLog);
    connect(m_symbolWidget, &SymbolWidget::setFocus, m_editorWidget, &ScintillaWidget::focusSet);
    connect(m_symbolWidget, &SymbolWidget::setIndex, m_editorWidget, &ScintillaWidget::indexSet);
    connect(m_symbolWidget, &SymbolWidget::fillIndicator, m_editorWidget, &ScintillaWidget::indicatorFill);

    QTimer::singleShot(0, this, [this] {
        // state
        breakpointGet();
        regionGet();
        // lsp
        didOpenNotification();
        contentChange();
        // logging
        emit appendLog(LogLevel::Info, "document opened", QString("<a href='%1'>%2</a>").arg(m_documentUrl.toString(), m_documentUrl.toString()));
    });
}

void LuaPage::propertySet(const QVariantMap &objects) {
    m_global = qvariant_cast<QObject *>(objects["global"]);
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    m_breakpointEditDialog = qvariant_cast<QObject *>(objects["breakpointModuleEditDialog"]);
    m_systemPropertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_editorMenu = qvariant_cast<QObject *>(objects["documentModuleEditorMenu"]);
    m_symbolWidget->propertySet(QVariantMap{
        {"global", QVariant::fromValue(m_global)},
        {"mainWindowToolTip", QVariant::fromValue(m_toolTip)}
    });
    m_searchWidget->propertySet(QVariantMap{
        {"global", QVariant::fromValue(m_global)},
        {"mainWindowToolTip", QVariant::fromValue(m_toolTip)}
    });
}

void LuaPage::themeLoad(const int theme) const {
    auto themeFile = QFile(QDir::current().filePath(QString("theme/%1.json").arg(QString::number(theme))));
    if (!themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    const auto themeData = themeFile.readAll();
    themeFile.close();
    const auto themeDoc = QJsonDocument::fromJson(themeData);
    const auto themeConfig = themeDoc.object();
    const auto styleConfig = themeConfig["style"].toObject();
    m_editorWidget->styleDefine(
        LuaTokenType::Namespace,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["namespace"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Class,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["class"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Type,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["type"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Parameter,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["parameter"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Variable,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["variable"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Property,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["property"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::EnumMember,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["enumMember"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::FunctionCall,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["functionCall"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::FunctionDeclaration,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["functionDeclaration"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Method,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["method"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Macro,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["macro"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Keyword,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["keyword"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Comment,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["comment"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::String,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["string"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Number,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["number"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
    m_editorWidget->styleDefine(
        LuaTokenType::Operator,
        QJsonObject{
            {"fore", ScintillaWidget::colorGet(styleConfig["operator"].toObject()["fore"].toString())},
            {"back", ScintillaWidget::colorGet(g_global->backGet())}
        });
}

QVariantHash LuaPage::menuGet(const QString &name) const {
    QVariantHash menuSession{};
    if (name == "edit") {
        menuSession = {
            {"undoable", m_editorWidget->undoable()},
            {"redoable", m_editorWidget->redoable()},
            {"copiable", m_editorWidget->copiable()},
            {"pastable", m_editorWidget->pastable()}
        };
    } else if (name == "nav") {
        menuSession = {
            {"documentUrl", m_documentUrl},
            {"line", m_selection["startLine"]},
            {"character", m_selection["character"]},
            {"navigation", navigable(m_editorWidget->currentPos())}
        };
    } else if (name == "code") {
        menuSession = {
            {"documentUrl", m_documentUrl},
            {"line", m_selection["line"]},
            {"character", m_selection["character"]},
            {"startLine", m_selection["startLine"]},
            {"startCharacter", m_selection["startCharacter"]},
            {"endLine", m_selection["endLine"]},
            {"endCharacter", m_selection["endCharacter"]},
            {"text", m_editorWidget->textGetSelected()}
        };
    } else if (name == "exec") {
        menuSession = {
            {"documentUrl", m_documentUrl},
            {"documentName", m_documentUrl.fileName()},
            {"startLine", m_selection["startLine"]},
            {"startCharacter", m_selection["startCharacter"]},
            {"endLine", m_selection["endLine"]},
            {"endCharacter", m_selection["endCharacter"]},
            {"text", m_editorWidget->textGetSelected()}
        };
    } else if (name == "editor") {
        menuSession = {
            {"documentUrl", m_documentUrl},
            {"line", m_selection["line"]},
            {"character", m_selection["character"]},
            {"startLine", m_selection["startLine"]},
            {"startCharacter", m_selection["startCharacter"]},
            {"endLine", m_selection["endLine"]},
            {"endCharacter", m_selection["endCharacter"]},
            {"text", m_editorWidget->textGetSelected()},
            {"navigation", navigable(m_editorWidget->currentPos())},
            {"assembly", m_assemblyWidget->isVisible()}
        };
    }
    return menuSession;
}

void LuaPage::menuRequest(const QString &request) {
    m_editorWidget->focusSet(true);
    if (request == "undo") {
        m_editorWidget->undo();
    } else if (request == "redo") {
        m_editorWidget->redo();
    } else if (request == "cut") {
        m_editorWidget->cut();
    } else if (request == "copy") {
        m_editorWidget->copy();
    } else if (request == "paste") {
        m_editorWidget->paste();
    } else if (request == "search") {
        searchToggle();
    } else if (request == "replace") {
        replaceToggle();
    }
}

// public: file
void LuaPage::documentSave() {
    if (!m_editorWidget->modifyGet()) return;
    // update status
    m_editorWidget->savepointSet();
    didSaveNotification();
    // save file
    const QString documentPath = m_documentUrl.toLocalFile();
    QFile file(documentPath);
    if (!file.open(QIODevice::WriteOnly)) return;
    QTextStream out(&file);
    out << m_editorWidget->textGet();
    file.close();
    // logging
    emit appendLog(LogLevel::Info, "document saved", QString("<a href='%1'>%2</a>").arg(m_documentUrl.toString(), m_documentUrl.toString()));
}

void LuaPage::permissionGet() {
    const QString documentPath = m_documentUrl.toLocalFile();
    const QFileInfo documentInfo(documentPath);
    m_editorWidget->readonlySet(!documentInfo.isWritable());
    BasePage::permissionGet();
}

// public: lsp
void LuaPage::diagnosticsNotification(const QJsonArray &diagnostics) {
    if (!m_documentUrl.toString().endsWith(".lua")) return;
    m_diagnostic = diagnostics;
    // clear
    m_editorWidget->indicatorClear(ScintillaIndicator::Password);
    m_editorWidget->indicatorClear(ScintillaIndicator::Error);
    m_editorWidget->indicatorClear(ScintillaIndicator::Warning);
    m_editorWidget->indicatorClear(ScintillaIndicator::Info);
    m_editorWidget->indicatorClear(ScintillaIndicator::Hint);
    // publish
    for (const auto &value: diagnostics) {
        const QJsonObject diagnostic = value.toObject();
        // placeholder operation
        int severity{};
        if (diagnostic["message"].toString().contains("__PLACEHOLDER__PASSWORD__")) {
            severity = 0;
        } else {
            severity = diagnostic["severity"].toInt();
        }
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const int startLine = start["line"].toInt();
        const int startCharacter = start["character"].toInt();
        const int endLine = end["line"].toInt();
        const int endCharacter = end["character"].toInt();
        int type{};
        switch (severity) {
            case 0: {
                type = ScintillaIndicator::Password;
            }
            break;
            case 1: {
                type = ScintillaIndicator::Error;
            }
            break;
            case 2: {
                type = ScintillaIndicator::Warning;
            }
            break;
            case 3: {
                type = ScintillaIndicator::Info;
            }
            break;
            case 4: {
                type = ScintillaIndicator::Hint;
            }
            break;
            default: break;
        }
        m_editorWidget->indicatorFill(type, startLine, startCharacter, endLine, endCharacter);
    }
}

void LuaPage::documentHighlightResponse(const QJsonArray &result) const {
    // clear previous highlight
    m_editorWidget->indicatorClear(ScintillaIndicator::Highlight);
    m_editorWidget->indicatorClear(ScintillaIndicator::Read);
    m_editorWidget->indicatorClear(ScintillaIndicator::Write);
    // highlight
    for (const auto &highlight: result) {
        const QJsonObject highlightObject = highlight.toObject();
        const int kind = highlightObject["kind"].toInt();
        const QJsonObject highlightRange = highlightObject["range"].toObject();
        const QJsonObject highlightStartPos = highlightRange["start"].toObject();
        const QJsonObject highlightEndPos = highlightRange["end"].toObject();
        const int startLine = highlightStartPos["line"].toInt();
        const int startCharacter = highlightStartPos["character"].toInt();
        const int endLine = highlightEndPos["line"].toInt();
        const int endCharacter = highlightEndPos["character"].toInt();
        m_editorWidget->indicatorFill(ScintillaIndicator::Highlight, startLine, startCharacter, endLine, endCharacter);
        if (kind == 2) m_editorWidget->indicatorFill(ScintillaIndicator::Read, startLine, startCharacter, endLine, endCharacter);
        else if (kind == 3) m_editorWidget->indicatorFill(ScintillaIndicator::Write, startLine, startCharacter, endLine, endCharacter);
    }
}

void LuaPage::documentSymbolResponse(const QJsonArray &result) {
    if (!m_documentUrl.toString().endsWith(".lua")) return;
    m_symbol = result;
}

void LuaPage::foldingRangeResponse(const QJsonArray &result) const {
    QHash<int, int> deltaDepthHash{};
    for (const auto &value: result) {
        const QJsonObject valueObject = value.toObject();
        const int startLine = valueObject["startLine"].toInt();
        const int endLine = valueObject["endLine"].toInt();
        deltaDepthHash.insert(startLine + 1, deltaDepthHash.value(startLine + 1, 0) + 1);
        deltaDepthHash.insert(endLine + 1, deltaDepthHash.value(endLine + 1, 0) - 1);
    }
    int depth = 0;
    for (int line = 0; line < m_editorWidget->lineCountGet(); line++) {
        const int deltaDepth = deltaDepthHash.value(line, 0);
        depth += deltaDepth;
        int level = SC_FOLDLEVELBASE + depth;
        if (deltaDepthHash.value(line + 1, 0) > 0) level |= SC_FOLDLEVELHEADERFLAG;
        m_editorWidget->foldLevelSet(line, level);
    }
}

void LuaPage::formattingResponse(const QString &newText) const {
    m_editorWidget->textSet(newText);
}

void LuaPage::onTypeFormattingResponse(const QJsonObject &newText) const {
    const QString text = newText["newText"].toString();
    const QJsonObject range = newText["range"].toObject();
    const QJsonObject start = range["start"].toObject();
    const QJsonObject end = range["end"].toObject();
    const int startLine = start["line"].toInt();
    const int startCharacter = start["character"].toInt();
    const int endLine = end["line"].toInt();
    const int endCharacter = end["character"].toInt();
    m_editorWidget->textSet(text, startLine, startCharacter, endLine, endCharacter);
}

void LuaPage::rangeFormattingResponse(const QString &newText) const {
    m_editorWidget->textSetSelected(newText);
}

void LuaPage::semanticTokensResponse(const QJsonArray &data) {
    // clear
    m_editorWidget->styleSet(LuaTokenType::Unused);
    // publish
    int line = 0;
    int character = 0;
    for (int i = 0; i < data.size(); i += 5) {
        const int deltaLine = data[i].toInt();
        const int deltaCharacter = data[i + 1].toInt();
        const int length = data[i + 2].toInt();
        const int tokenType = data[i + 3].toInt();
        const int tokenModifiers = data[i + 4].toInt();
        line += deltaLine;
        character = deltaLine > 0 ? deltaCharacter : character + deltaCharacter;
        int type{};
        switch (tokenType) {
            case LspTokenType::Namespace:
                type = LuaTokenType::Namespace;
                break;
            case LspTokenType::Class:
                type = LuaTokenType::Class;
                break;
            case LspTokenType::Type:
                type = LuaTokenType::Type;
                break;
            case LspTokenType::Parameter:
                type = LuaTokenType::Parameter;
                break;
            case LspTokenType::Variable:
                type = LuaTokenType::Variable;
                break;
            case LspTokenType::Property:
                type = LuaTokenType::Property;
                break;
            case LspTokenType::EnumMember:
                type = LuaTokenType::EnumMember;
                break;
            case LspTokenType::Function:
                if (tokenModifiers == LspTokenModifiers::Declaration || tokenModifiers == LspTokenModifiers::Global) type = LuaTokenType::FunctionDeclaration;
                else type = LuaTokenType::FunctionCall;
                break;
            case LspTokenType::Method:
                type = LuaTokenType::Method;
                break;
            case LspTokenType::Macro:
                type = LuaTokenType::Macro;
                break;
            case LspTokenType::Keyword:
                type = LuaTokenType::Keyword;
                break;
            case LspTokenType::Comment:
                type = LuaTokenType::Comment;
                break;
            case LspTokenType::String:
                type = LuaTokenType::String;
                break;
            case LspTokenType::Number:
                type = LuaTokenType::Number;
                break;
            case LspTokenType::Operator:
                type = LuaTokenType::Operator;
                break;
            default:
                emit appendLog(LogLevel::Warning, "contact author:", QString("unsupported semantic (token type:%1)").arg(QString::number(tokenType)));
                break;
        }
        m_editorWidget->styleSet(type, line, character, length);
    }
}

// public: typo
void LuaPage::spellCheckResponse(const QVariantList &typos) {
    m_typo = typos;
    // clear
    m_editorWidget->indicatorClear(ScintillaIndicator::Typo);
    // publish
    for (const auto &value: typos) {
        auto typo = value.toMap();
        const int startLine = typo["line"].toInt();
        const int endLine = typo["line"].toInt();
        const int startCharacter = typo["startCharacter"].toInt();
        const int endCharacter = typo["endCharacter"].toInt();
        m_editorWidget->indicatorFill(ScintillaIndicator::Typo, startLine, startCharacter, endLine, endCharacter);
    }
}

// public: slot
void LuaPage::charAdd(const int ch) {
    m_contentTimer->start();
    m_selection = m_editorWidget->selectionGet();
    const QChar character(ch);
    if (character.isLetter() || m_completionSet.contains(character)) {
        didChangeNotification();
        completionRequest();
    } else if (m_signatureHelpSet.contains(character)) {
        didChangeNotification();
        completionRequest();
        signatureHelpRequest();
    } else if (m_onTypeFormattingSet.contains(character)) {
        didChangeNotification();
        onTypeFormattingRequest();
    }
}

void LuaPage::assemblyToggle(const bool status) {
    if (status) {
        QTemporaryFile tempFile;
        if (!tempFile.open()) return;
        tempFile.write(m_editorWidget->textGet().toUtf8());
        tempFile.close();
        QProcess process;
        process.start("luac", QStringList() << "-l" << tempFile.fileName());
        // process.start("luac", QStringList() << "-l" << "-l" << tempFile.fileName());
        if (!process.waitForFinished(1000)) return;
        const auto error = QString::fromUtf8(process.readAllStandardError());
        m_assemblyWidget->readonlySet(false);
        if (!error.isEmpty()) {
            m_assemblyWidget->textSet(error);
        } else {
            m_editorWidget->annotationClear();
            m_editorWidget->markerDelete(ScintillaMarker::Navigation);
            m_l2aHash.clear();
            m_assemblyWidget->textClear();
            // m_assemblyWidget->textSet(process.readAllStandardOutput());
            const auto output = QString::fromUtf8(process.readAllStandardOutput()).split("\r\n");
            QString type{};
            int startLine{};
            int endLine{};
            for (const auto &text: output) {
                if (text.isEmpty()) {
                    if (m_assemblyWidget->lineCountGet() == 1) continue;
                    m_assemblyWidget->textAppend("\r\n");
                } else if (text.contains('<') && text.contains('>')) {
                    // parse type from space
                    const auto tmp0 = text.indexOf(' ');
                    type = text.mid(0, tmp0);
                    // parse range from :x,y>
                    const auto tmp1 = text.lastIndexOf(':');
                    const auto tmp2 = text.lastIndexOf(',');
                    const auto tmp3 = text.lastIndexOf('>');
                    startLine = text.mid(tmp1 + 1, tmp2 - tmp1 - 1).toInt();
                    endLine = text.mid(tmp2 + 1, tmp3 - tmp2 - 1).toInt();
                    m_assemblyWidget->textAppend(QString("%1 %2-%3\r\n").arg(type, QString::number(startLine), QString::number(endLine)));
                    m_assemblyWidget->foldLevelSet(m_assemblyWidget->lineCountGet() - 3, SC_FOLDLEVELBASE);
                    m_assemblyWidget->foldLevelSet(m_assemblyWidget->lineCountGet() - 2, SC_FOLDLEVELBASE + SC_FOLDLEVELHEADERFLAG);
                    // 1 based
                    if (startLine > 1) {
                        startLine--;
                        m_editorWidget->markerAdd(ScintillaMarker::Navigation, startLine);
                        m_l2aHash.insert(startLine, m_assemblyWidget->lineCountGet() - 2);
                    }
                    if (endLine > 1) endLine--;
                } else if (text.contains("param")
                           && text.contains("slot")
                           && text.contains("upvalue")
                           && text.contains("local")
                           && text.contains("constant")
                           && text.contains("function")) {
                    if (text.startsWith("0+")) continue;
                    m_editorWidget->annotationSet(startLine, text);
                } else {
                    const auto detail = text.split('\t');
                    if (detail.size() == 6) m_assemblyWidget->textAppend(detail.at(3) + '\t' + detail.at(4) + '\t' + detail.at(5) + "\r\n");
                    else m_assemblyWidget->textAppend(detail.at(3) + '\t' + detail.at(4) + "\r\n");
                    m_assemblyWidget->marginTextSet(m_assemblyWidget->lineCountGet() - 2, detail.at(1));
                    m_assemblyWidget->foldLevelSet(m_assemblyWidget->lineCountGet() - 2, SC_FOLDLEVELBASE + 1);
                }
            }
        }
        m_assemblyWidget->readonlySet(true);
        m_assemblyWidget->show();
    } else {
        m_editorWidget->annotationClear();
        m_editorWidget->markerDelete(ScintillaMarker::Navigation);
        m_l2aHash.clear();
        m_assemblyWidget->textClear();
        m_assemblyWidget->hide();
    }
}

bool LuaPage::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_editorWidget->viewport()) {
        const QPoint globalPos = QCursor::pos();
        const QPoint localPos = m_editorWidget->viewport()->mapFromGlobal(globalPos);
        if (event->type() == QEvent::MouseButtonPress) {
            m_dwellTimer->stop();
            const auto *mouseEvent = static_cast<QMouseEvent *>(event);
            const auto modifiers = mouseEvent->modifiers();
            const auto position = m_editorWidget->positionGet(localPos);
            const auto index = m_editorWidget->indexGet(position);
            // margin click
            if (localPos.x() < m_editorWidget->marginWidthGet()) return false;
            // text area click
            if (mouseEvent->button() == Qt::LeftButton) {
                if (modifiers == Qt::ControlModifier) {
                    m_editorWidget->positionSet(position);
                    if (m_editorWidget->indicatorGet(position) & 1 << ScintillaIndicator::Hyperlink) {
                        emit requestDefinition(m_documentUrl, index["line"], index["character"]);
                        emit requestReferences(m_documentUrl, index["line"], index["character"]);
                    }
                    return false;
                }
            }
            if (mouseEvent->button() == Qt::RightButton) {
                if (m_editorWidget->textGetSelected().isEmpty()) m_editorWidget->positionSet(position);
                QMetaObject::invokeMethod(m_editorMenu, "popup");
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            const auto *mouseEvent = static_cast<QMouseEvent *>(event);
            const auto modifiers = mouseEvent->modifiers();
            if (modifiers == Qt::ControlModifier) {
                m_dwellTimer->stop();
                const auto position = m_editorWidget->positionGet(localPos);
                navigationToggle(position);
                return true;
            }
            m_dwellTimer->start();
            navigationToggle();
            return false;
        }
    }
    if (watched == m_editorWidget) {
        if (event->type() == QEvent::KeyPress) {
            m_dwellTimer->stop();
            const auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_QuoteDbl:
                    symbolPair('"');
                    return false;
                case Qt::Key_Apostrophe:
                    symbolPair('\'');
                    return false;
                case Qt::Key_ParenLeft:
                    symbolPair('(');
                    return false;
                case Qt::Key_BracketLeft:
                    symbolPair('[');
                    return false;
                case Qt::Key_BraceLeft:
                    symbolPair('{');
                    return false;
                default: return false;
            }
        }
    }
    return DockWidget::eventFilter(watched, event);
}

// void LuaPage::documentClose() {
//     if () {
//     m_saveDialog->setProperty("documentUrl", m_documentUrl);
//     m_saveDialog->setProperty("documentName", m_documentUrl.fileName());
//     QMetaObject::invokeMethod(m_saveDialog, "open");
//     }
// }

// private: slot
void LuaPage::marginClick(const Scintilla::Position position, const int mouseButton, const Scintilla::KeyMod modifiers, const int margin) {
    const int line = m_editorWidget->lineGet(position);
    if (margin == 1) {
        if (mouseButton == Qt::LeftButton) {
            if (m_editorWidget->markerGet(line) & 1 << ScintillaMarker::Region) {
                for (int current = line; current < m_editorWidget->lineCountGet(); ++current) {
                    const QString text = m_editorWidget->textGet(current, 0, current, -1);
                    if (text.contains("--#endregion")) {
                        emit startThread(m_documentUrl, InterpreterMode::Run, line + 1, 0, current - 1, -1);
                        return;
                    }
                }
                qDebug() << "error: --#endregion not found";
            } else if (m_editorWidget->markerGet(line) & 1 << ScintillaMarker::BreakpointEnabled) {
                emit removeBreakpoint(m_documentUrl, line + 1);
                m_editorWidget->markerDelete(ScintillaMarker::BreakpointEnabled, line);
            } else if (m_editorWidget->markerGet(line) & 1 << ScintillaMarker::BreakpointDisabled) {
                emit removeBreakpoint(m_documentUrl, line + 1);
                m_editorWidget->markerDelete(ScintillaMarker::BreakpointDisabled, line);
            } else if (m_editorWidget->markerGet(line) & 1 << ScintillaMarker::Navigation) {
                m_assemblyWidget->markerAdd(ScintillaMarker::Hint, m_l2aHash[line], 1000);
            } else {
                emit insertBreakpoint(m_documentUrl, line + 1, QVariantHash({
                                          {"condition", ""},
                                          {"enabled", true}
                                      }));
                if (modifiers == Scintilla::KeyMod::Ctrl) breakpointSet(line + 1);
            }
        } else if (mouseButton == Qt::RightButton) {
            if (m_editorWidget->markerGet(line) & 1 << ScintillaMarker::BreakpointEnabled || m_editorWidget->markerGet(line) & 1 << ScintillaMarker::BreakpointDisabled) {
                breakpointSet(line + 1);
            }
        }
    }
}

void LuaPage::selectionChange() {
    m_selection = m_editorWidget->selectionGet();
    emit changeSelection(m_selection);
    if (m_selection["lines"] == 0 && m_selection["characters"] == 0) {
        documentHighlightRequest();
        m_symbolWidget->symbolLoad(m_symbol, m_selection["line"], m_selection["character"]);
    }
}

void LuaPage::contentChange() {
    m_selection = m_editorWidget->selectionGet();
    // status refresh
    breakpointGet();
    regionGet();
    // lsp request
    didChangeNotification();
    documentHighlightRequest();
    documentSymbolRequest();
    foldingRangeRequest();
    semanticTokensRequest();
    // nuspell request
    spellCheckRequest();
    // assembly request
    assemblyToggle(m_assemblyWidget->isVisible());
}

void LuaPage::dwellChange() {
    hoverRequest();
}

void LuaPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}

// private: file
void LuaPage::permissionSet() const {
    m_systemPropertyDialog->setProperty("fileUrl", m_documentUrl);
    QMetaObject::invokeMethod(m_systemPropertyDialog, "open");
}

void LuaPage::breakpointGet() const {
    m_editorWidget->markerDelete(ScintillaMarker::BreakpointEnabled);
    m_editorWidget->markerDelete(ScintillaMarker::BreakpointDisabled);
    if (g_breakpoints.contains(m_documentUrl)) {
        for (const auto &line: g_breakpoints[m_documentUrl].keys()) {
            if (g_breakpoints[m_documentUrl][line]["enabled"].toBool()) m_editorWidget->markerAdd(ScintillaMarker::BreakpointEnabled, line - 1);
            else m_editorWidget->markerAdd(ScintillaMarker::BreakpointDisabled, line - 1);
        }
    }
}

void LuaPage::breakpointSet(const int line) const {
    m_breakpointEditDialog->setProperty("documentUrl", m_documentUrl);
    m_breakpointEditDialog->setProperty("line", line);
    QMetaObject::invokeMethod(m_breakpointEditDialog, "open");
}

void LuaPage::regionGet() const {
    m_editorWidget->markerDelete(ScintillaMarker::Region);
    for (int line = 0; line < m_editorWidget->lineCountGet(); ++line) {
        const QString text = m_editorWidget->textGet(line, 0, line, -1);
        if (text.contains("--#region")) {
            m_editorWidget->markerAdd(ScintillaMarker::Region, line);
        }
    }
}

// private: lsp
void LuaPage::didOpenNotification() {
    // did open notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_documentUrl.toString()},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", m_editorWidget->textGet()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void LuaPage::didChangeNotification() {
    // did change notification to lua language server
    const auto content = m_editorWidget->textGet();
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_documentUrl.toString()},
                {"version", m_version++}
            }
        },
        {
            "contentChanges", QJsonArray{
                QJsonObject{
                    {"text", content}
                }
            }
        }
    };
    emit notificationJson("textDocument/didChange", didChangeParams);
}

void LuaPage::didSaveNotification() {
    // did save notification to lua language server
    const QJsonObject didSaveParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_documentUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didSave", didSaveParams);
}

void LuaPage::didCloseNotification() {
    // did close notification to lua language server
    const QJsonObject didCloseParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_documentUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didClose", didCloseParams);
}

void LuaPage::completionRequest() {
    // completion request to script module
    emit requestCompletion(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::definitionRequest() {
    // definition request to script module
    emit requestDefinition(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::documentHighlightRequest() {
    // document highlight request to script module
    emit requestDocumentHighlight(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::documentSymbolRequest() {
    // document symbol request to script module
    emit requestDocumentSymbol(m_documentUrl);
}

void LuaPage::foldingRangeRequest() {
    // folding range request to script module
    emit requestFoldingRange(m_documentUrl);
}

void LuaPage::formattingRequest() {
    // formatting request to script module
    emit requestFormatting(m_documentUrl);
}

void LuaPage::hoverRequest() {
    const auto globalPos = QCursor::pos();
    const auto localPos = m_editorWidget->mapFromGlobal(globalPos);
    if (!rect().contains(localPos)) return;
    const auto closePosition = m_editorWidget->closePositionGet(localPos);
    if (closePosition == -1) return;
    const auto index = m_editorWidget->indexGet(closePosition);
    const auto line = index["line"];
    const auto character = index["character"];
    if (line == 0 && character == 0) return;
    // show diagnostic if exists
    QString diagnosticText = "<table width='100%'>";
    for (const auto &value: m_diagnostic) {
        const QJsonObject diagnostic = value.toObject();
        const QJsonObject range = diagnostic["range"].toObject();
        const QJsonObject start = range["start"].toObject();
        const QJsonObject end = range["end"].toObject();
        const int startLine = start["line"].toInt();
        const int startCharacter = start["character"].toInt();
        const int endLine = end["line"].toInt();
        const int endCharacter = end["character"].toInt();
        if (line < startLine || line > endLine) continue;
        if (line == startLine && character < startCharacter) continue;
        if (line == endLine && character > endCharacter) continue;
        const int severity = diagnostic["severity"].toInt();
        QString severityString{};
        switch (severity) {
            case 1: {
                severityString = "Error";
            }
            break;
            case 2: {
                severityString = "Warning";
            }
            break;
            case 3: {
                severityString = "Info";
            }
            break;
            case 4: {
                severityString = "Hint";
            }
            break;
            default: break;
        }
        const QString message = diagnostic["message"].toString();
        // qDebug() << message << parsed;
        const QString customUrl = QString("request.code.action://reserved/%1/%2/%3/%4").arg(
            QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter));
        diagnosticText += QString("<tr><td><b>%1</b>: %2</td><td align='right'><a href='%3'>Code Action</a></td></tr>").arg(severityString, md2html(message), customUrl);
    }
    // show typo if exists
    for (const auto &value: m_typo) {
        auto typo = value.toMap();
        const int startLine = typo["line"].toInt();
        const int endLine = typo["line"].toInt();
        const int startCharacter = typo["startCharacter"].toInt();
        const int endCharacter = typo["endCharacter"].toInt();
        if (line < startLine || line > endLine) continue;
        if (line == startLine && character < startCharacter) continue;
        if (line == endLine && character > endCharacter) continue;
        const QString word = m_editorWidget->textGet(startLine, startCharacter, endLine, endCharacter);
        const QString customUrl = QString("request.spell.suggestion://%1/%2/%3/%4/%5").arg(
            word, QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter));
        diagnosticText += QString("<tr><td><b>Typo</b>: In word '%1'</td><td align='right'><a href='%2'>Show Suggestions</a></td></tr>").arg(word, customUrl);
    }
    // call diagnostic show
    const QVariantHash diagnosticSession = {
        {"documentUrl", m_documentUrl},
        {"position", QCursor::pos()}
    };
    if (diagnosticText == "<table width='100%'>") {
        emit showDiagnostic(diagnosticSession, "");
    } else {
        diagnosticText += "</table>";
        emit showDiagnostic(diagnosticSession, diagnosticText);
    }
    // hover request to script module
    emit requestHover(m_documentUrl, line, character);
}

void LuaPage::implementationRequest() {
    // implementation request to script module
    emit requestImplementation(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::referencesRequest() {
    // references request to script module
    emit requestReferences(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::onTypeFormattingRequest() {
    // on type formatting request to script module
    emit requestOnTypeFormatting(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::semanticTokensRequest() {
    // semantic tokens request to script module
    emit requestSemanticTokens(m_documentUrl);
}

void LuaPage::signatureHelpRequest() {
    // signature help request to script module
    emit requestSignatureHelp(m_documentUrl, m_selection["line"], m_selection["character"]);
}

void LuaPage::typeDefinitionRequest() {
    // type definition request to script module
    emit requestTypeDefinition(m_documentUrl, m_selection["line"], m_selection["character"]);
}

// private: typo
void LuaPage::spellCheckRequest() {
    if (!m_documentUrl.toString().endsWith(".lua")) return;
    // spell check request to script module
    emit requestSpellCheck(m_documentUrl, m_editorWidget->textGet());
}

// private: misc
void LuaPage::commentToggle() {
    if (m_editorWidget->textGetSelected().isEmpty()) {
        const auto position = m_editorWidget->positionGet();
        const auto index = m_editorWidget->indexGet(position);
        auto text = m_editorWidget->textGet(index["line"], 0, index["line"], -1);
        if (text.contains("--")) {
            text.remove("--");
        } else {
            text = "--" + text;
        }
        m_editorWidget->textSet(text, index["line"], 0, index["line"], -1);
    } else {
        auto text = m_editorWidget->textGetSelected();
        if (text.contains("--[[") || text.contains("]]")) {
            text.remove("--[[");
            text.remove("]]");
        } else {
            text = "--[[" + text + "]]";
        }
        m_editorWidget->textSetSelected(text);
    }
    contentChange();
}

bool LuaPage::navigable(const Scintilla::Position position) const {
    const int type = m_editorWidget->styleGet(position);
    if (type > 0 && type < LuaTokenType::Macro) return true;
    return false;
}

void LuaPage::navigationToggle(const Scintilla::Position position) const {
    if (position == -1) {
        m_editorWidget->indicatorClear(ScintillaIndicator::Hyperlink);
        m_toolTip->setProperty("text", "");
    } else {
        if (navigable(position)) {
            const auto wordIndex = m_editorWidget->wordIndexGet(position);
            m_editorWidget->indicatorFill(ScintillaIndicator::Hyperlink, wordIndex["startLine"], wordIndex["startCharacter"], wordIndex["endLine"], wordIndex["endCharacter"]);
            m_toolTip->setProperty("position", QCursor::pos());
            m_toolTip->setProperty("text", tr("Click to navigate"));
        } else {
            m_editorWidget->indicatorClear(ScintillaIndicator::Hyperlink);
            m_toolTip->setProperty("text", "");
        }
    }
}

void LuaPage::symbolPair(const QChar character) {
    auto selected = m_editorWidget->textGetSelected();
    if (selected.isEmpty()) {
        selected = m_pairHash[character];
        m_editorWidget->textSetSelected(selected);
        const auto position = m_editorWidget->positionGet();
        m_editorWidget->positionSet(position - 1);
    } else {
        selected = character + selected + m_pairHash[character];
        m_editorWidget->textSetSelected(selected);
    }
    contentChange();
}

// private: search
void LuaPage::searchToggle() {
    if (m_selection["characters"] != 0) {
        m_searchWidget->show();
        m_searchWidget->searchRequest(m_editorWidget->textGetSelected());
    } else {
        m_searchWidget->setVisible(!m_searchWidget->isVisible());
    }
}

void LuaPage::replaceToggle() {
    if (m_selection["characters"] != 0) {
        m_searchWidget->show();
        m_searchWidget->searchRequest(m_editorWidget->textGetSelected());
    } else {
        m_searchWidget->setVisible(!m_searchWidget->isVisible());
    }
}

void LuaPage::searchRequest(const QString &text) {
    searchClear();
    m_search["text"] = text;
    if (!text.isEmpty()) {
        int current = 0;
        int total = 0;
        QVariantList startList{};
        QVariantList endList{};

        m_editorWidget->targetSetWhole();
        while (true) {
            if (m_editorWidget->targetSearch(text) == -1) break;
            const auto start = m_editorWidget->targetGetStart();
            const auto end = m_editorWidget->targetGetEnd();
            startList.append(start);
            endList.append(end);
            const auto startIndex = m_editorWidget->indexGet(start);
            const auto endIndex = m_editorWidget->indexGet(end);
            m_editorWidget->indicatorFill(
                ScintillaIndicator::Search,
                startIndex["line"],
                startIndex["character"],
                endIndex["line"],
                endIndex["character"]
            );
            if (m_selection["startPosition"] > start) current++;
            m_editorWidget->targetSetStart(end);
            m_editorWidget->targetSetEnd(m_editorWidget->lengthGet());
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

void LuaPage::searchResponse() {
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
    const auto startIndex = m_editorWidget->indexGet(startList[current].toInt());
    const auto endIndex = m_editorWidget->indexGet(endList[current].toInt());
    m_editorWidget->indexSet(
        startIndex["line"],
        startIndex["character"]
    );
    m_editorWidget->indicatorFill(
        ScintillaIndicator::Selection,
        startIndex["line"],
        startIndex["character"],
        endIndex["line"],
        endIndex["character"]
    );
}

void LuaPage::searchPrev() {
    m_editorWidget->indicatorClear(ScintillaIndicator::Selection);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != 0) {
        m_search["current"] = current - 1;
    } else {
        m_search["current"] = total - 1;
    }
    searchResponse();
}

void LuaPage::searchNext() {
    m_editorWidget->indicatorClear(ScintillaIndicator::Selection);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != total - 1) {
        m_search["current"] = current + 1;
    } else {
        m_search["current"] = 0;
    }
    searchResponse();
}

void LuaPage::searchClear() {
    m_search.clear();
    m_editorWidget->indicatorClear(ScintillaIndicator::Search);
    m_editorWidget->indicatorClear(ScintillaIndicator::Selection);
}

void LuaPage::textReplace(const QString &text) {
    const auto current = m_search["current"].toInt();
    const auto startList = m_search["start"].toList();
    const auto endList = m_search["end"].toList();
    const auto startIndex = m_editorWidget->indexGet(startList[current].toInt());
    const auto endIndex = m_editorWidget->indexGet(endList[current].toInt());
    m_editorWidget->indexSet(
        startIndex["line"],
        startIndex["character"]
    );
    m_editorWidget->textSet(
        text,
        startIndex["line"],
        startIndex["character"],
        endIndex["line"],
        endIndex["character"]
    );
    m_searchWidget->searchRequest();
}

void LuaPage::allReplace(const QString &text) {
    m_editorWidget->undoBegin();
    for (int index = m_search["total"].toInt() - 1; index >= 0; --index) {
        const auto startList = m_search["start"].toList();
        const auto endList = m_search["end"].toList();
        const auto startIndex = m_editorWidget->indexGet(startList[index].toInt());
        const auto endIndex = m_editorWidget->indexGet(endList[index].toInt());
        m_editorWidget->textSet(
            text,
            startIndex["line"],
            startIndex["character"],
            endIndex["line"],
            endIndex["character"]
        );
    }
    m_editorWidget->undoEnd();
    m_searchWidget->searchRequest();
}
