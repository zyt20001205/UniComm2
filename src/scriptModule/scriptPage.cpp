#include "scriptModule/scriptPage.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QTemporaryFile>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "scriptModule/codeAnalysis/symbolWidget.h"
#include "scriptModule/codeEditor/scintillaWidget.h"
#include "scriptModule/codeEditor/searchWidget.h"
#include "utils/cmarkUtils.h"

// public
ScriptPage::ScriptPage(const QJsonObject &scriptConfig, const QUrl &scriptUrl)
    : DockWidget(scriptUrl.toString()),
      m_scriptUrl(scriptUrl),
      m_editorWidget(new ScintillaWidget(this)),
      m_selectionTimer(new QTimer(this)),
      m_contentTimer(new QTimer(this)),
      m_dwellTimer(new QTimer(this)),
      m_symbolWidget(new SymbolWidget(this)),
      m_assemblyWidget(new ScintillaWidget(this)),
      m_fileWatcher(new QFileSystemWatcher()),
      m_searchWidget(new SearchWidget()),
      m_completionSet{'.', ':', '\'', '"', '[', '#', '*', '@', '|', '=', '-', '{', '+', '?'},
      m_signatureHelpSet{'(', ','},
      m_onTypeFormattingSet{'\n'},
      m_pairHash{{'"', '"'}, {'\'', '\''}, {'(', ')'}, {'[', ']'}, {'{', '}'}} {
    setTitle(scriptUrl.fileName());
    auto shortcutLineDuplicate = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this); // NOLINT
    connect(shortcutLineDuplicate, &QShortcut::activated, m_editorWidget, &ScintillaWidget::lineDuplicate);
    shortcutLineDuplicate->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    connect(shortcutSearch, &QShortcut::activated, m_searchWidget, &SearchWidget::toggle);
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutComment = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash), this); // NOLINT
    connect(shortcutComment, &QShortcut::activated, this, &ScriptPage::commentToggle);
    shortcutComment->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutFormatting = new QShortcut(QKeySequence(scriptConfig["formatting"].toString()), this); // NOLINT
    connect(shortcutFormatting, &QShortcut::activated, this, &ScriptPage::formattingRequest);
    shortcutFormatting->setContext(Qt::WidgetWithChildrenShortcut);

    // 100ms debounce for selection change
    m_selectionTimer->setSingleShot(true);
    m_selectionTimer->setInterval(100);
    connect(m_selectionTimer, &QTimer::timeout, this, &ScriptPage::selectionChange);
    // 500ms debounce for content change
    m_contentTimer->setSingleShot(true);
    m_contentTimer->setInterval(500);
    connect(m_contentTimer, &QTimer::timeout, this, &ScriptPage::contentChange);
    // 1000ms debounce for dwell change
    m_dwellTimer->setSingleShot(true);
    m_dwellTimer->setInterval(1000);
    connect(m_dwellTimer, &QTimer::timeout, this, &ScriptPage::dwellChange);

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

            m_editorWidget->send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xffffff); // NOLINT

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
                m_editorWidget->send(SCI_MARKERSETFORE, i, 0xffffff); // NOLINT
                m_editorWidget->send(SCI_MARKERSETBACK, i, 0x000000); // NOLINT
            }
            m_editorWidget->send(SCI_SETFOLDMARGINCOLOUR, true, 0xffffff); // NOLINT
            m_editorWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, 0xffffff); // NOLINT
            m_editorWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
            m_editorWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
            m_editorWidget->send(SCI_STYLESETBACK, STYLE_FOLDDISPLAYTEXT, 0xe0e0e0); // NOLINT
            // TODO: hotspot is not working for STYLE_FOLDDISPLAYTEXT
            // m_editorWidget->send(SCI_STYLESETHOTSPOT, STYLE_FOLDDISPLAYTEXT, true); // NOLINT

            m_editorWidget->send(SCI_SETUSETABS, false); // NOLINT
            m_editorWidget->send(SCI_SETINDENT, 4); // NOLINT
            m_editorWidget->send(SCI_SETTABINDENTS, true); // NOLINT
            m_editorWidget->send(SCI_SETBACKSPACEUNINDENTS, true); // NOLINT
            m_editorWidget->send(SCI_SETINDENTATIONGUIDES, SC_IV_REAL); // NOLINT

            m_editorWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
            m_editorWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT

            m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, 0x80ffd2a6); // NOLINT
            m_editorWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            m_editorWidget->send(SCI_SETCARETLINEVISIBLE, true); // NOLINT
            m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, 0x80fef8f5); // NOLINT
            m_editorWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            // for debug
            // m_editorWidget->send(SCI_SETVIEWEOL, true); // NOLINT
            // script
            const QUrl &url(scriptUrl);
            const QString scriptPath = url.toLocalFile();
            QFile file(scriptPath);
            if (!file.open(QIODevice::ReadOnly)) return;
            QTextStream in(&file);
            const QString script = in.readAll();
            file.close();
            m_editorWidget->textSet(script);
            // history
            m_editorWidget->send(SCI_EMPTYUNDOBUFFER); // NOLINT
            m_editorWidget->send(SCI_SETCHANGEHISTORY,SC_CHANGE_HISTORY_ENABLED | SC_CHANGE_HISTORY_MARKERS); // NOLINT
        }
        // font
        m_editorWidget->fontSet(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
        // indicator
        {
            m_editorWidget->indicatorDefine(
                INDICATOR_TYPO,
                QJsonObject{
                    {"style", 14},
                    {"fore", 0xabd180},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_HINT,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0xf5f5f5},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_INFO,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0xfaf0e6},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_WARNING,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0xe6f5ff},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_ERROR,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0xe6e6ff},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_PASSWORD,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0x000000},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", false}
                });
            // QJsonObject{
            //             {"style", 1},
            //             {"fore", 0x1f0fc5},
            //             {"alpha", 255},
            //             {"outlineAlpha", 255},
            //             {"setUnder", true}
            // });
            m_editorWidget->indicatorDefine(
                INDICATOR_HIGHLIGHT,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0xe0e0e0},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_READ,
                QJsonObject{
                    {"style", 17},
                    {"fore", 0xb85f00},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_WRITE,
                QJsonObject{
                    {"style", 17},
                    {"fore", 0x2828c6},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_SEARCH,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0x7ed4fc},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_SELECTION,
                QJsonObject{
                    {"style", 8},
                    {"fore", 0x3372c4},
                    {"alpha", 255},
                    {"outlineAlpha", 255},
                    {"setUnder", true}
                });
            m_editorWidget->indicatorDefine(
                INDICATOR_HYPERLINK,
                QJsonObject{
                    {"style", 17},
                    {"fore", 0xcc6d00},
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
                    {"width", 32}
                });
            m_editorWidget->marginDefine(
                1,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(~SC_MASK_FOLDERS & ~SC_MASK_HISTORY)},
                    {"sensitive", true}
                });
            m_editorWidget->marginDefine(
                2,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(SC_MASK_FOLDERS)},
                    {"sensitive", true}
                });
            m_editorWidget->marginDefine(
                3,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 4},
                    {"mask", SC_MASK_HISTORY},
                });
        }
        // marker
        {
            m_editorWidget->markerDefine(
                MARKER_REGION,
                QJsonObject{
                    {"symbol", 2},
                    {"fore", 0x107c10},
                    {"back", 0x9fd89f}
                });
            m_editorWidget->markerDefine(
                MARKER_BREAKPOINT_ENABLED,
                QJsonObject{
                    {"symbol", 0},
                    {"fore", 0x1f0fc5},
                    {"back", 0xb2acee}
                });
            m_editorWidget->markerDefine(
                MARKER_BREAKPOINT_DISABLED,
                QJsonObject{
                    {"symbol", 0},
                    {"fore", 0x1f0fc5},
                    {"back", 0xffffff}
                });
            m_editorWidget->markerDefine(
                MARKER_NAVIGATION,
                QJsonObject{
                    {"symbol", 24},
                    {"fore", 0x000000},
                    {"back", 0x000000}
                });
            m_editorWidget->markerDefine(
                MARKER_DEBUG,
                QJsonObject{
                    {"symbol", 2},
                    {"fore", 0x000000},
                    {"back", 0xa500ff}
                });
            m_editorWidget->markerDefine(
                MARKER_ERROR,
                QJsonObject{
                    {"symbol", 22},
                    {"fore", 0xffe6e6},
                    {"back", 0xffe6e6}
                });
            m_editorWidget->markerDefine(
                MARKER_HINT,
                QJsonObject{
                    {"symbol", 22},
                    {"fore", 0xe0e0e0},
                    {"back", 0xe0e0e0}
                });
        }
        // style
        {
            m_editorWidget->styleDefine(
                LUA_TOKEN_NAMESPACE,
                QJsonObject{
                    {"fore", 0x808000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_CLASS,
                QJsonObject{
                    {"fore", 0x808000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_TYPE,
                QJsonObject{
                    {"fore", 0xb33300}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_PARAMETER,
                QJsonObject{
                    {"fore", 0x000000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_VARIABLE,
                QJsonObject{
                    {"fore", 0x000000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_PROPERTY,
                QJsonObject{
                    {"fore", 0x7a0e66}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_ENUMMEMBAER,
                QJsonObject{
                    {"fore", 0x941087}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_FUNCTION_DECLARATION,
                QJsonObject{
                    {"fore", 0x7a6200}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_FUNCTION_CALL,
                QJsonObject{
                    {"fore", 0x000000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_METHOD,
                QJsonObject{
                    {"fore", 0x000000}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_MACRO,
                QJsonObject{
                    {"fore", 0x2e541f}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_KEYWORD,
                QJsonObject{
                    {"fore", 0xb33300}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_COMMENT,
                QJsonObject{
                    {"fore", 0x8c8c8c}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_STRING,
                QJsonObject{
                    {"fore", 0x177d06}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_NUMBER,
                QJsonObject{
                    {"fore", 0xeb5017}
                });
            m_editorWidget->styleDefine(
                LUA_TOKEN_OPERATOR,
                QJsonObject{
                    {"fore", 0x000000}
                });
            m_editorWidget->styleDefine(
                STYLE_ANNOTATION,
                QJsonObject{
                    {"fore", 0x8c8c8c}
                });
        }
        // signals
        connect(m_editorWidget, &ScintillaEdit::modifyAttemptReadOnly, this, [this] { emit setPermission(m_scriptUrl, !m_editorWidget->readonlyGet()); });
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
        connect(m_editorWidget, &ScintillaEdit::charAdded, this, &ScriptPage::charAdd);
        connect(m_editorWidget, &ScintillaEdit::updateUi, this, [this](const Scintilla::Update updated) {
            if (updated == Scintilla::Update::Selection) m_selectionTimer->start();
        });
        connect(m_editorWidget, &ScintillaEdit::savePointChanged, this, &ScriptPage::savepointChange);
        connect(m_editorWidget, &ScintillaEdit::modified, this, [this](const Scintilla::ModificationFlags type) {
            if (static_cast<int>(type) & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT | SC_PERFORMED_UNDO | SC_PERFORMED_REDO)) m_contentTimer->start();
        });
        connect(m_editorWidget, &ScintillaEdit::hotSpotClick, this, [this](Scintilla::Position position, Scintilla::KeyMod modifiers) {
            qDebug() << "hotspot triggered!";
        });
        m_editorWidget->installEventFilter(this);
        m_editorWidget->viewport()->installEventFilter(this);
    }
    // assembly init
    {
        m_assemblyWidget->hide();
        // misc
        {
            m_assemblyWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
            m_assemblyWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT

            m_assemblyWidget->send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xffffff); // NOLINT

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
                m_assemblyWidget->send(SCI_MARKERSETFORE, i, 0xffffff); // NOLINT
                m_assemblyWidget->send(SCI_MARKERSETBACK, i, 0x000000); // NOLINT
            }
            m_assemblyWidget->send(SCI_SETFOLDMARGINCOLOUR, true, 0xffffff); // NOLINT
            m_assemblyWidget->send(SCI_SETFOLDMARGINHICOLOUR, true, 0xffffff); // NOLINT
            m_assemblyWidget->send(SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_STANDARD); // NOLINT
            m_assemblyWidget->send(SCI_SETDEFAULTFOLDDISPLAYTEXT, 0, reinterpret_cast<sptr_t>("...")); // NOLINT
            m_assemblyWidget->send(SCI_STYLESETBACK, STYLE_FOLDDISPLAYTEXT, 0xe0e0e0); // NOLINT
            // TODO: hotspot is not working for STYLE_FOLDDISPLAYTEXT
            // m_editorWidget->send(SCI_STYLESETHOTSPOT, STYLE_FOLDDISPLAYTEXT, true); // NOLINT

            m_assemblyWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
            m_assemblyWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT

            m_assemblyWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, 0x80ffd2a6); // NOLINT
            m_assemblyWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
            m_assemblyWidget->send(SCI_SETCARETLINEVISIBLE, true); // NOLINT
            m_assemblyWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, 0x80fef8f5); // NOLINT
            m_assemblyWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
        }
        // font
        m_assemblyWidget->fontSet(QFont(scriptConfig["fontFamily"].toString(), scriptConfig["fontSize"].toInt()));
        // margin
        {
            m_assemblyWidget->marginDefine(
                0,
                QJsonObject{
                    {"type", SC_MARGIN_TEXT},
                    {"width", 32}
                });
            m_assemblyWidget->marginDefine(
                1,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(~SC_MASK_FOLDERS)},
                    {"sensitive", true}
                });
            m_assemblyWidget->marginDefine(
                2,
                QJsonObject{
                    {"type", SC_MARGIN_SYMBOL},
                    {"width", 16},
                    {"mask", static_cast<int>(SC_MASK_FOLDERS)},
                    {"sensitive", true}
                });
        }
        // marker
        {
            m_assemblyWidget->markerDefine(
                MARKER_NAVIGATION,
                QJsonObject{
                    {"symbol", 24},
                    {"fore", 0x00ffff},
                    {"back", 0x00ffff}
                });
            m_assemblyWidget->markerDefine(
                MARKER_HINT,
                QJsonObject{
                    {"symbol", 22},
                    {"fore", 0xe0e0e0},
                    {"back", 0xe0e0e0}
                });
        }
        // style
        {
            m_assemblyWidget->styleDefine(
                STYLE_ANNOTATION,
                QJsonObject{
                    {"fore", 0x8c8c8c}
                });
        }
    }

    // layout->addWidget(m_searchWidget);
    layout->addWidget(codingwidget);
    layout->addWidget(m_symbolWidget);
    connect(m_symbolWidget, &SymbolWidget::setFocus, m_editorWidget, &ScintillaWidget::focusSet);
    connect(m_symbolWidget, &SymbolWidget::setIndex, m_editorWidget, &ScintillaWidget::indexSet);
    connect(m_symbolWidget, &SymbolWidget::fillIndicator, m_editorWidget, &ScintillaWidget::indicatorFill);

    QTimer::singleShot(0, this, [this] {
        // state
        permissionLoad();
        breakpointLoad();
        regionLoad();
        // lsp
        didOpenNotification();
        contentChange();
        // widgets
        m_symbolWidget->propertySet(QVariantMap{
            {"mainWindowTooltip", QVariant::fromValue(m_toolTip)}
        });
        // logging
        emit appendLog(QString("<a href='%1'>%2</a> opened").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        qDebug() << QString("[%1] %2 opened").arg(timestamp, m_scriptUrl.toString());
    });
}

// public: file
void ScriptPage::pathDisambiguation() {
    const QString scriptPath = m_scriptUrl.toLocalFile();
    const QString workspacePath = g_workspaceUrl.toLocalFile();
    const QString relatedPath = QDir(workspacePath).relativeFilePath(scriptPath);
    setTitle(relatedPath);
}

void ScriptPage::scriptReload() {
    // const QMessageBox::StandardButton reply = QMessageBox::question(
    //     nullptr,
    //     tr("Reload"),
    //     QString(tr("%1\n\n"
    //         "This file has been modified by another program.\n"
    //         "Do you want to reload it?")).arg(m_scriptUrl.toString()),
    //     QMessageBox::Yes | QMessageBox::No);
    // if (reply != QMessageBox::Yes) {
    //     return;
    // }
    // // reload new script
    // const QUrl &url(m_scriptUrl);
    // const QString scriptPath = url.toLocalFile();
    // QFile file(scriptPath);
    // file.open(QIODevice::ReadOnly);
    // QTextStream in(&file);
    // const QString content = in.readAll();
    // file.close();
    // m_editorWidget->setText(content);
    // // logging
    // emit appendLog(QString("<a href='%1'>%2</a> reloaded").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
    // QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    // qDebug() << QString("[%1] %2 reloaded").arg(timestamp, m_scriptUrl.fileName());
}

void ScriptPage::scriptSave() {
    if (!m_editorWidget->modifyGet()) return;
    // update status
    m_editorWidget->savepointSet();
    didSaveNotification();
    // block file watcher signals
    m_fileWatcher->blockSignals(true);
    // save file
    const QString scriptPath = m_scriptUrl.toLocalFile();
    QFile file(scriptPath);
    if (!file.open(QIODevice::WriteOnly)) return;
    QTextStream out(&file);
    out << m_editorWidget->textGet();
    file.close();
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> saved").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 saved").arg(timestamp, m_scriptUrl.toString());
    // restore file watcher signals 1 sec later
    QTimer::singleShot(1000, this, [this] { m_fileWatcher->blockSignals(false); });
}

void ScriptPage::scriptClose() {
    // ask for saving
    if (m_editorWidget->modifyGet()) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr,
            tr("Close Script"),
            tr("The script has been edited. Save changes?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            scriptSave();
        }
    }
    didCloseNotification();
    emit closeScript(m_scriptUrl);
    deleteLater();
    // logging
    emit appendLog(QString("<a href='%1'>%2</a> closed").arg(m_scriptUrl.toString(), m_scriptUrl.toString()), "info");
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    qDebug() << QString("[%1] %2 closed").arg(timestamp, m_scriptUrl.toString());
}

// public: lsp
void ScriptPage::diagnosticsResponse(const QJsonArray &diagnostics) {
    if (!m_scriptUrl.toString().endsWith(".lua")) return;
    m_diagnostic = diagnostics;
    // clear
    m_editorWidget->indicatorClear(INDICATOR_PASSWORD);
    m_editorWidget->indicatorClear(INDICATOR_ERROR);
    m_editorWidget->indicatorClear(INDICATOR_WARNING);
    m_editorWidget->indicatorClear(INDICATOR_INFO);
    m_editorWidget->indicatorClear(INDICATOR_HINT);
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
                type = INDICATOR_PASSWORD;
            }
            break;
            case 1: {
                type = INDICATOR_ERROR;
            }
            break;
            case 2: {
                type = INDICATOR_WARNING;
            }
            break;
            case 3: {
                type = INDICATOR_INFO;
            }
            break;
            case 4: {
                type = INDICATOR_HINT;
            }
            break;
            default: break;
        }
        m_editorWidget->indicatorFill(type, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::documentHighlightResponse(const QJsonArray &result) const {
    // clear previous highlight
    m_editorWidget->indicatorClear(INDICATOR_HIGHLIGHT);
    m_editorWidget->indicatorClear(INDICATOR_READ);
    m_editorWidget->indicatorClear(INDICATOR_WRITE);
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
        m_editorWidget->indicatorFill(INDICATOR_HIGHLIGHT, startLine, startCharacter, endLine, endCharacter);
        if (kind == 2) m_editorWidget->indicatorFill(INDICATOR_READ, startLine, startCharacter, endLine, endCharacter);
        else if (kind == 3) m_editorWidget->indicatorFill(INDICATOR_WRITE, startLine, startCharacter, endLine, endCharacter);
    }
}

void ScriptPage::documentSymbolResponse(const QJsonArray &result) {
    if (!m_scriptUrl.toString().endsWith(".lua")) return;
    m_symbol = result;
}

void ScriptPage::foldingRangeResponse(const QJsonArray &result) const {
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

void ScriptPage::formattingResponse(const QString &newText) const {
    m_editorWidget->textSet(newText);
}

void ScriptPage::onTypeFormattingResponse(const QJsonObject &newText) const {
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

void ScriptPage::rangeFormattingResponse(const QString &newText) const {
    m_editorWidget->textSetSelected(newText);
}

void ScriptPage::semanticTokensResponse(const QJsonArray &data) const {
    // clear
    m_editorWidget->styleSet(LUA_TOKEN_UNUSED);
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
            case TOKENTYPE_NAMESPACE:
                type = LUA_TOKEN_NAMESPACE;
                break;
            case TOKENTYPE_CLASS:
                type = LUA_TOKEN_CLASS;
                break;
            case TOKENTYPE_TYPE:
                type = LUA_TOKEN_TYPE;
                break;
            case TOKENTYPE_PARAMETER:
                type = LUA_TOKEN_PARAMETER;
                break;
            case TOKENTYPE_VARIABLE:
                type = LUA_TOKEN_VARIABLE;
                break;
            case TOKENTYPE_PROPERTY:
                type = LUA_TOKEN_PROPERTY;
                break;
            case TOKENTYPE_ENUMMEMBAER:
                type = LUA_TOKEN_ENUMMEMBAER;
                break;
            case TOKENTYPE_FUNCTION:
                if (tokenModifiers == TOKENMODIFIERS_DECLARATION || tokenModifiers == TOKENMODIFIERS_GLOBAL) type = LUA_TOKEN_FUNCTION_DECLARATION;
                else type = LUA_TOKEN_FUNCTION_CALL;
                break;
            case TOKENTYPE_METHOD:
                type = LUA_TOKEN_METHOD;
                break;
            case TOKENTYPE_MACRO:
                type = LUA_TOKEN_MACRO;
                break;
            case TOKENTYPE_KEYWORD:
                type = LUA_TOKEN_KEYWORD;
                break;
            case TOKENTYPE_COMMENT:
                type = LUA_TOKEN_COMMENT;
                break;
            case TOKENTYPE_STRING:
                type = LUA_TOKEN_STRING;
                break;
            case TOKENTYPE_NUMBER:
                type = LUA_TOKEN_NUMBER;
                break;
            case TOKENTYPE_OPERATOR:
                type = LUA_TOKEN_OPERATOR;
                break;
            default:
                qDebug() << "skip token" << line << character << length << tokenType;
                break;
        }
        m_editorWidget->styleSet(type, line, character, length);
    }
}

// public: typo
void ScriptPage::spellCheckResponse(const QVariantList &typos) {
    m_typo = typos;
    // clear
    m_editorWidget->indicatorClear(INDICATOR_TYPO);
    // publish
    for (const auto &value: typos) {
        auto typo = value.toMap();
        const int startLine = typo["line"].toInt();
        const int endLine = typo["line"].toInt();
        const int startCharacter = typo["startCharacter"].toInt();
        const int endCharacter = typo["endCharacter"].toInt();
        m_editorWidget->indicatorFill(INDICATOR_TYPO, startLine, startCharacter, endLine, endCharacter);
    }
}

// public: slot
void ScriptPage::charAdd(const int ch) {
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

void ScriptPage::assemblyToggle(const bool status) {
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
                        m_editorWidget->markerAdd(MARKER_NAVIGATION, startLine);
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
                    if (detail.length() == 6) m_assemblyWidget->textAppend(detail.at(3) + '\t' + detail.at(4) + '\t' + detail.at(5) + "\r\n");
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
        m_l2aHash.clear();
        m_assemblyWidget->textClear();
        m_assemblyWidget->hide();
    }
}

// protected
void ScriptPage::closeEvent(QCloseEvent *event) {
    scriptClose();
    event->accept();
}

bool ScriptPage::eventFilter(QObject *watched, QEvent *event) {
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
                    if (m_editorWidget->indicatorGet(position) & 1 << INDICATOR_HYPERLINK) {
                        emit requestDefinition(m_scriptUrl, index["line"], index["character"]);
                        emit requestReferences(m_scriptUrl, index["line"], index["character"]);
                    }
                    return false;
                }
            }
            if (mouseEvent->button() == Qt::RightButton) {
                bool navigation = false;
                bool rangeFormatting = false;
                QString text{};
                text = m_editorWidget->textGetSelected();
                if (text.isEmpty()) {
                    m_editorWidget->positionSet(position);
                } else {
                    rangeFormatting = true;
                }
                const int type = m_editorWidget->styleGet(position);
                if (type > 0 && type < LUA_TOKEN_MACRO) navigation = true;
                const QVariantHash menuSession = {
                    {"navigation", navigation},
                    {"rangeFormatting", rangeFormatting},
                    {"assembly", m_assemblyWidget->isVisible()},
                    {"line", index["line"]},
                    {"character", index["character"]},
                    {"startLine", m_selection["startLine"]},
                    {"startCharacter", m_selection["startCharacter"]},
                    {"endLine", m_selection["endLine"]},
                    {"endCharacter", m_selection["endCharacter"]},
                    {"text", text}
                };
                emit showMenu(m_scriptUrl, menuSession);
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

// private: slot
void ScriptPage::marginClick(const Scintilla::Position position, const int mouseButton, const Scintilla::KeyMod modifiers, const int margin) {
    const int line = m_editorWidget->lineGet(position);
    if (margin == 1) {
        if (mouseButton == Qt::LeftButton) {
            if (m_editorWidget->markerGet(line) & 1 << MARKER_REGION) {
                for (int current = line; current < m_editorWidget->lineCountGet(); ++current) {
                    const QString text = m_editorWidget->textGet(current, 0, current, -1);
                    if (text.contains("--#endregion")) {
                        emit startThread(m_scriptUrl, LUATHREAD_RUN, line + 1, 0, current - 1, -1);
                        return;
                    }
                }
                qDebug() << "error: --#endregion not found";
            } else if (m_editorWidget->markerGet(line) & 1 << MARKER_BREAKPOINT_ENABLED) {
                emit removeBreakpoint(m_scriptUrl, line + 1);
                m_editorWidget->markerDelete(MARKER_BREAKPOINT_ENABLED, line);
            } else if (m_editorWidget->markerGet(line) & 1 << MARKER_BREAKPOINT_DISABLED) {
                emit removeBreakpoint(m_scriptUrl, line + 1);
                m_editorWidget->markerDelete(MARKER_BREAKPOINT_DISABLED, line);
            } else if (m_editorWidget->markerGet(line) & 1 << MARKER_NAVIGATION) {
                m_assemblyWidget->markerAdd(MARKER_HINT, m_l2aHash[line], 1000);
            } else {
                emit insertBreakpoint(m_scriptUrl, line + 1, QVariantHash({
                                          {"condition", ""},
                                          {"enabled", true}
                                      }));
                m_editorWidget->markerAdd(MARKER_BREAKPOINT_ENABLED, line);
                if (modifiers == Scintilla::KeyMod::Ctrl) emit editBreakpoint(m_scriptUrl, line + 1);
            }
        } else if (mouseButton == Qt::RightButton) {
            if (m_editorWidget->markerGet(line) & 1 << MARKER_BREAKPOINT_ENABLED || m_editorWidget->markerGet(line) & 1 << MARKER_BREAKPOINT_DISABLED) {
                emit editBreakpoint(m_scriptUrl, line + 1);
            }
        }
    }
}

void ScriptPage::selectionChange() {
    m_selection = m_editorWidget->selectionGet();
    emit changeSelection(m_selection);
    if (m_selection["lines"] == 0 && m_selection["characters"] == 0) {
        documentHighlightRequest();
        m_symbolWidget->symbolLoad(m_symbol, m_selection["line"], m_selection["character"]);
    }
}

void ScriptPage::contentChange() {
    m_selection = m_editorWidget->selectionGet();
    // status refresh
    breakpointLoad();
    regionLoad();
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

void ScriptPage::dwellChange() {
    hoverRequest();
}

void ScriptPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}

// private: file
void ScriptPage::permissionLoad() {
    const QString scriptPath = m_scriptUrl.toLocalFile();
    const QFileInfo fileInfo(scriptPath);
    if (fileInfo.isWritable()) {
        setIcon(QIcon());
        m_editorWidget->readonlySet(false);
    } else {
        setIcon(QIcon(":/icon/lockClosed.svg"));
        m_editorWidget->readonlySet(true);
    }
}

// void ScriptPage::permissionRequest() {
//     // block file watcher signals
//     m_fileWatcher->blockSignals(true);
//     const QString scriptPath = m_scriptUrl.toLocalFile();
//     QFile::setPermissions(
//         scriptPath,
//         QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
//     // logging
//     emit appendLog(QString("<a href='%1'>%2</a> permitted").arg(m_scriptUrl.toString(), m_scriptUrl.fileName()), "info");
//     QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
//     qDebug() << QString("[%1] %2 permitted").arg(timestamp, m_scriptUrl.fileName());
//     // restore file watcher signals 1 sec later
//     QTimer::singleShot(1000, this, [this] { m_fileWatcher->blockSignals(false); });
// }

void ScriptPage::breakpointLoad() const {
    m_editorWidget->markerDelete(MARKER_BREAKPOINT_ENABLED);
    m_editorWidget->markerDelete(MARKER_BREAKPOINT_DISABLED);
    if (g_breakpoints.contains(m_scriptUrl)) {
        for (const auto &line: g_breakpoints[m_scriptUrl].keys()) {
            if (g_breakpoints[m_scriptUrl][line]["enabled"].toBool()) m_editorWidget->markerAdd(MARKER_BREAKPOINT_ENABLED, line - 1);
            else m_editorWidget->markerAdd(MARKER_BREAKPOINT_DISABLED, line - 1);
        }
    }
}

void ScriptPage::regionLoad() const {
    m_editorWidget->markerDelete(MARKER_REGION);
    for (int line = 0; line < m_editorWidget->lineCountGet(); ++line) {
        const QString text = m_editorWidget->textGet(line, 0, line, -1);
        if (text.contains("--#region")) {
            m_editorWidget->markerAdd(MARKER_REGION, line);
        }
    }
}

// private: lsp
void ScriptPage::didOpenNotification() {
    // did open notification to lua language server
    const QJsonObject didOpenParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
                {"languageId", "lua"},
                {"version", m_version++},
                {"text", m_editorWidget->textGet()}
            }
        }
    };
    emit notificationJson("textDocument/didOpen", didOpenParams);
}

void ScriptPage::didChangeNotification() {
    // did change notification to lua language server
    const auto content = m_editorWidget->textGet();
    const QJsonObject didChangeParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()},
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

void ScriptPage::didSaveNotification() {
    // did save notification to lua language server
    const QJsonObject didSaveParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didSave", didSaveParams);
}

void ScriptPage::didCloseNotification() {
    // did close notification to lua language server
    const QJsonObject didCloseParams{
        {
            "textDocument", QJsonObject{
                {"uri", m_scriptUrl.toString()}
            }
        }
    };
    emit notificationJson("textDocument/didClose", didCloseParams);
}

void ScriptPage::completionRequest() {
    // completion request to script module
    emit requestCompletion(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::definitionRequest() {
    // definition request to script module
    emit requestDefinition(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::documentHighlightRequest() {
    // document highlight request to script module
    emit requestDocumentHighlight(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::documentSymbolRequest() {
    // document symbol request to script module
    emit requestDocumentSymbol(m_scriptUrl);
}

void ScriptPage::foldingRangeRequest() {
    // folding range request to script module
    emit requestFoldingRange(m_scriptUrl);
}

void ScriptPage::formattingRequest() {
    // formatting request to script module
    emit requestFormatting(m_scriptUrl);
}

void ScriptPage::hoverRequest() {
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
        const QString commandLine = QString("requestcodeaction://codeAction/%2/%3/%4/%5").arg(
            QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter));
        diagnosticText += QString("<tr><td><b>%1</b>: %2</td><td align='right'><a href='%3'>Code Action</a></td></tr>").arg(severityString, md2html(message), commandLine);
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
        const QString commandLine = QString("requestspellsuggest://%1/%2/%3/%4/%5").arg(
            word, QString::number(startLine), QString::number(startCharacter), QString::number(endLine), QString::number(endCharacter));
        diagnosticText += QString("<tr><td><b>Typo</b>: In word '%1'</td><td align='right'><a href='%2'>Show Suggestions</a></td></tr>").arg(word, commandLine);
    }
    // call diagnostic show
    const QPoint position = m_editorWidget->window()->mapFromGlobal(QCursor::pos() + QPoint(10, 10));
    const QVariantHash diagnosticSession = {
        {"scriptUrl", m_scriptUrl},
        {"position", position}
    };
    if (diagnosticText == "<table width='100%'>") {
        emit showDiagnostic(diagnosticSession, "");
    } else {
        diagnosticText += "</table>";
        emit showDiagnostic(diagnosticSession, diagnosticText);
    }
    // hover request to script module
    emit requestHover(m_scriptUrl, line, character);
}

void ScriptPage::implementationRequest() {
    // implementation request to script module
    emit requestImplementation(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::referencesRequest() {
    // references request to script module
    emit requestReferences(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::onTypeFormattingRequest() {
    // on type formatting request to script module
    emit requestOnTypeFormatting(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::semanticTokensRequest() {
    // semantic tokens request to script module
    emit requestSemanticTokens(m_scriptUrl);
}

void ScriptPage::signatureHelpRequest() {
    // signature help request to script module
    emit requestSignatureHelp(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

void ScriptPage::typeDefinitionRequest() {
    // type definition request to script module
    emit requestTypeDefinition(m_scriptUrl, m_selection["line"], m_selection["character"]);
}

// private: typo
void ScriptPage::spellCheckRequest() {
    if (!m_scriptUrl.toString().endsWith(".lua")) return;
    // spell check request to script module
    emit requestSpellCheck(m_scriptUrl, m_editorWidget->textGet());
}

// private:
void ScriptPage::commentToggle() {
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

void ScriptPage::symbolPair(const QChar character) {
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

void ScriptPage::navigationToggle(const Scintilla::Position position) {
    if (position == -1) {
        m_editorWidget->indicatorClear(INDICATOR_HYPERLINK);
        m_toolTip->setProperty("text", "");
    } else {
        const int type = m_editorWidget->styleGet(position);
        if (type > 0 && type < LUA_TOKEN_MACRO) {
            const auto wordIndex = m_editorWidget->wordIndexGet(position);
            m_editorWidget->indicatorFill(INDICATOR_HYPERLINK, wordIndex["startLine"], wordIndex["startCharacter"], wordIndex["endLine"], wordIndex["endCharacter"]);
            m_toolTip->setProperty("position", QCursor::pos());
            m_toolTip->setProperty("text", tr("Click to navigate"));
        } else {
            m_editorWidget->indicatorClear(INDICATOR_HYPERLINK);
            m_toolTip->setProperty("text", "");
        }
    }
}

void ScriptPage::positionFill(const int x, const int y) const {
    // const QString text = QString("%1, %2").arg(QString::number(x), QString::number(y));
    // m_editorWidget->insert(text);
    // const long currentPos = m_editorWidget->SendScintilla(SCI_GETCURRENTPOS);
    // const long cursorPos = currentPos + text.length();
    // m_editorWidget->SendScintilla(SCI_SETCURRENTPOS, cursorPos); // NOLINT
    // m_editorWidget->SendScintilla(SCI_SETSELECTIONSTART, cursorPos); // NOLINT
    // m_editorWidget->SendScintilla(SCI_SETSELECTIONEND, cursorPos); // NOLINT
}
