#include "document/page/textPage.h"

#include <QDir>
#include <QFileInfo>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>

#include "globals.h"
#include "document/module/replaceWidget.h"
#include "document/module/scintillaWidget.h"
#include "document/module/searchWidget.h"

// public
TextPage::TextPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_editorWidget(new ScintillaWidget(this)),
      m_searchWidget(new SearchWidget(this)),
      m_replaceWidget(new ReplaceWidget(this)),
      m_selectionTimer(new QTimer(this)) {
    auto shortcutSearch = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this); // NOLINT
    connect(shortcutSearch, &QShortcut::activated, this, &TextPage::searchToggle);
    shortcutSearch->setContext(Qt::WidgetWithChildrenShortcut);
    auto shortcutReplace = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this); // NOLINT
    connect(shortcutReplace, &QShortcut::activated, this, &TextPage::replaceToggle);
    shortcutReplace->setContext(Qt::WidgetWithChildrenShortcut);

    // 100ms debounce for selection change
    m_selectionTimer->setSingleShot(true);
    m_selectionTimer->setInterval(100);
    connect(m_selectionTimer, &QTimer::timeout, this, &TextPage::selectionChange);

    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // misc
    {
        m_editorWidget->send(SCI_SETSCROLLWIDTH, 1); // NOLINT
        m_editorWidget->send(SCI_SETSCROLLWIDTHTRACKING, true); // NOLINT

        m_editorWidget->send(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xffffff); // NOLINT

        // m_editorWidget->send(SCI_STYLESETHOTSPOT, STYLE_FOLDDISPLAYTEXT, true); // NOLINT

        m_editorWidget->send(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT
        m_editorWidget->send(SCI_EOLANNOTATIONSETVISIBLE, ANNOTATION_STANDARD); // NOLINT

        m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_SELECTION_BACK, 0x80ffd2a6); // NOLINT
        m_editorWidget->send(SCI_SETSELECTIONLAYER, SC_LAYER_UNDER_TEXT); // NOLINT
        m_editorWidget->send(SCI_SETELEMENTCOLOUR, SC_ELEMENT_CARET_LINE_BACK, 0x80fef8f5); // NOLINT
        m_editorWidget->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT); // NOLINT
    }
    // font
    m_editorWidget->fontSet(QFont(documentConfig["fontFamily"].toString(), documentConfig["fontSize"].toInt()));
    // indicator
    {
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
    }
    // margin
    {
        m_editorWidget->marginDefine(
            0,
            QJsonObject{
                {"type", SC_MARGIN_TEXT},
                {"width", 32}
            });
        m_editorWidget->marginDefine(
            3,
            QJsonObject{
                {"type", SC_MARGIN_SYMBOL},
                {"width", 4},
                {"mask", SC_MASK_HISTORY},
            });
    }
    // load
    const QUrl &url(documentUrl);
    const QString documentPath = url.toLocalFile();
    QFile file(documentPath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QTextStream in(&file);
    const QString text = in.readAll();
    file.close();
    m_editorWidget->textSet(text);

    connect(m_editorWidget, &ScintillaEdit::modifyAttemptReadOnly, this, &TextPage::permissionSet);
    connect(m_editorWidget, &ScintillaEdit::savePointChanged, this, &TextPage::savepointChange);

    layout->addWidget(m_searchWidget);
    layout->addWidget(m_replaceWidget);
    layout->addWidget(m_editorWidget);
}

void TextPage::propertySet(const QVariantMap &objects) {
    m_toolTip = qvariant_cast<QObject *>(objects["mainWindowToolTip"]);
    m_systemPropertyDialog = qvariant_cast<QObject *>(objects["fileModulePropertyDialog"]);
    m_searchWidget->propertySet(QVariantMap{
        {"mainWindowToolTip", QVariant::fromValue(m_toolTip)}
    });
    m_replaceWidget->propertySet(QVariantMap{
        {"mainWindowToolTip", QVariant::fromValue(m_toolTip)}
    });
}

void TextPage::documentSave() {
    // TODO: waiting for filewatcher
}

// protected
void TextPage::closeEvent(QCloseEvent *event) {
    documentClose();
    DockWidget::closeEvent(event);
}

void TextPage::selectionChange() {
    m_selection = m_editorWidget->selectionGet();
    emit changeSelection(m_selection);
}

// private: slot
void TextPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}

void TextPage::permissionSet() const {
    m_systemPropertyDialog->setProperty("fileUrl", m_documentUrl);
    QMetaObject::invokeMethod(m_systemPropertyDialog, "open");
}

// private: search
void TextPage::searchToggle() {
    if (m_selection["characters"] != 0) {
        m_searchWidget->show();
        m_replaceWidget->hide();
        m_searchWidget->searchRequest(m_editorWidget->textGetSelected());
    } else {
        m_searchWidget->setVisible(!m_searchWidget->isVisible());
        m_replaceWidget->hide();
    }
}

void TextPage::replaceToggle() {
    if (m_selection["characters"] != 0) {
        m_searchWidget->show();
        m_replaceWidget->show();
        m_searchWidget->searchRequest(m_editorWidget->textGetSelected());
    } else {
        m_replaceWidget->setVisible(!m_replaceWidget->isVisible());
        m_searchWidget->setVisible(m_replaceWidget->isVisible());
    }
}

void TextPage::searchRequest(const QString &text) {
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
                INDICATOR_SEARCH,
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

void TextPage::searchResponse() {
    if (m_search["total"].toInt() == 0) {
        m_searchWidget->searchEnable(false);
        m_replaceWidget->replaceEnable(false);
        m_searchWidget->searchResponse("0/0");
        return;
    }
    m_searchWidget->searchEnable(true);
    m_replaceWidget->replaceEnable(true);
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
        INDICATOR_SELECTION,
        startIndex["line"],
        startIndex["character"],
        endIndex["line"],
        endIndex["character"]
    );
}

void TextPage::searchPrev() {
    m_editorWidget->indicatorClear(INDICATOR_SELECTION);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != 0) {
        m_search["current"] = current - 1;
    } else {
        m_search["current"] = total - 1;
    }
    searchResponse();
}

void TextPage::searchNext() {
    m_editorWidget->indicatorClear(INDICATOR_SELECTION);
    const auto current = m_search["current"].toInt();
    const auto total = m_search["total"].toInt();
    if (current != total - 1) {
        m_search["current"] = current + 1;
    } else {
        m_search["current"] = 0;
    }
    searchResponse();
}

void TextPage::searchClear() {
    m_search.clear();
    m_editorWidget->indicatorClear(INDICATOR_SEARCH);
    m_editorWidget->indicatorClear(INDICATOR_SELECTION);
}

void TextPage::textReplace(const QString &text) {
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

void TextPage::allReplace(const QString &text) {
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
