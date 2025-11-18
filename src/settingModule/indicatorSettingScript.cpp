#include "settingModule/indicatorSettingScript.h"

#include <QColorDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "Qsci/qsciscintilla.h"

#include "globals.h"

// IndicatorSettingScript public
IndicatorSettingScript::IndicatorSettingScript(QWidget *parent)
    : QScrollArea(parent),
      m_indicatorStyleList{
          tr("Plain"), tr("Squiggle"), tr("TT"), tr("Diagonal"), tr("Strike"), tr("Hidden"), tr("Box"), tr("Round Box"), tr("Straight Box"), tr("Dashes"),
          tr("Dots"), tr("Squiggle Low"), tr("Dot Box"), tr("Squiggle Pixmap"), tr("Thick Composition"), tr("Thin Composition"), tr("Full Box"), tr("Text Color"),
          tr("Triangle"), tr("Triangle Character"), tr("Gradient"), tr("Centre Gradient")
      },
      m_indicatorErrorStyleComboBox(new QComboBox()),
      m_indicatorErrorColorButton(new QPushButton()),
      m_indicatorWarningStyleComboBox(new QComboBox()),
      m_indicatorWarningColorButton(new QPushButton()),
      m_indicatorInfoStyleComboBox(new QComboBox()),
      m_indicatorInfoColorButton(new QPushButton()),
      m_indicatorHintStyleComboBox(new QComboBox()),
      m_indicatorHintColorButton(new QPushButton()),
      m_diagnosticPreviewEditor(new QsciScintilla()),
      m_indicatorHighlightStyleComboBox(new QComboBox()),
      m_indicatorHighlightColorButton(new QPushButton()),
      m_indicatorReadStyleComboBox(new QComboBox()),
      m_indicatorReadColorButton(new QPushButton()),
      m_indicatorWriteStyleComboBox(new QComboBox()),
      m_indicatorWriteColorButton(new QPushButton()),
      m_highlightPreviewEditor(new QsciScintilla()),
      m_indicatorSearchStyleComboBox(new QComboBox()),
      m_indicatorSearchColorButton(new QPushButton()),
      m_indicatorSelectionStyleComboBox(new QComboBox()),
      m_indicatorSelectionColorButton(new QPushButton()),
      m_searchPreviewEditor(new QsciScintilla()),
      m_indicatorHyperlinkStyleComboBox(new QComboBox()),
      m_indicatorHyperlinkColorButton(new QPushButton()),
      m_hyperlinkPreviewEditor(new QsciScintilla()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    setWidgetResizable(true);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    // diagnostic
    {
        auto *diagnosticLabel = new QLabel(tr("Diagnostic")); // NOLINT
        layout->addWidget(diagnosticLabel);
        diagnosticLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(HLine);
        horizontalLine->setLineWidth(3);

        auto *indicatorErrorStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorErrorStyleWidget);
        auto *indicatorErrorStyleLayout = new QHBoxLayout(indicatorErrorStyleWidget); // NOLINT
        indicatorErrorStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorErrorStyleLabel = new QLabel(tr("Error Style")); // NOLINT
        indicatorErrorStyleLayout->addWidget(indicatorErrorStyleLabel);
        indicatorErrorStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorErrorStyleLayout->addWidget(m_indicatorErrorStyleComboBox);
        m_indicatorErrorStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorErrorStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorErrorStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_ERROR);
            m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorErrorColorButton->text()), INDICATOR_ERROR);
            m_diagnosticPreviewEditor->fillIndicatorRange(0, 0, 0, m_diagnosticPreviewEditor->text(0).length(), INDICATOR_ERROR);
        });
        auto *indicatorErrorColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorErrorColorWidget);
        auto *indicatorErrorColorLayout = new QHBoxLayout(indicatorErrorColorWidget); // NOLINT
        indicatorErrorColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorErrorColorLabel = new QLabel(tr("Error Color")); // NOLINT
        indicatorErrorColorLayout->addWidget(indicatorErrorColorLabel, 1);
        indicatorErrorColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorErrorColorLayout->addWidget(m_indicatorErrorColorButton, 1);
        m_indicatorErrorColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorErrorColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorErrorColorButton->text(), this, tr("Choose Error Background")); newColor.isValid()) {
                m_indicatorErrorColorButton->setText(newColor.name());
                m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_ERROR);
                m_diagnosticPreviewEditor->fillIndicatorRange(0, 0, 0, m_diagnosticPreviewEditor->text(0).length(), INDICATOR_ERROR);
            }
        });

        auto *indicatorWarningStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorWarningStyleWidget);
        auto *indicatorWarningStyleLayout = new QHBoxLayout(indicatorWarningStyleWidget); // NOLINT
        indicatorWarningStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorWarningStyleLabel = new QLabel(tr("Warning Style")); // NOLINT
        indicatorWarningStyleLayout->addWidget(indicatorWarningStyleLabel);
        indicatorWarningStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorWarningStyleLayout->addWidget(m_indicatorWarningStyleComboBox);
        m_indicatorWarningStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorWarningStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorWarningStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_WARNING);
            m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorWarningColorButton->text()), INDICATOR_WARNING);
            m_diagnosticPreviewEditor->fillIndicatorRange(1, 0, 1, m_diagnosticPreviewEditor->text(0).length(), INDICATOR_WARNING);
        });
        auto *indicatorWarningColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorWarningColorWidget);
        auto *indicatorWarningColorLayout = new QHBoxLayout(indicatorWarningColorWidget); // NOLINT
        indicatorWarningColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorWarningColorLabel = new QLabel(tr("Warning Color")); // NOLINT
        indicatorWarningColorLayout->addWidget(indicatorWarningColorLabel, 1);
        indicatorWarningColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorWarningColorLayout->addWidget(m_indicatorWarningColorButton, 1);
        m_indicatorWarningColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorWarningColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorWarningColorButton->text(), this, tr("Choose Warning Background")); newColor.isValid()) {
                m_indicatorWarningColorButton->setText(newColor.name());
                m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_WARNING);
                m_diagnosticPreviewEditor->fillIndicatorRange(1, 0, 1, m_diagnosticPreviewEditor->text(1).length(), INDICATOR_WARNING);
            }
        });

        auto *indicatorInfoStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorInfoStyleWidget);
        auto *indicatorInfoStyleLayout = new QHBoxLayout(indicatorInfoStyleWidget); // NOLINT
        indicatorInfoStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorInfoStyleLabel = new QLabel(tr("Info Style")); // NOLINT
        indicatorInfoStyleLayout->addWidget(indicatorInfoStyleLabel);
        indicatorInfoStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorInfoStyleLayout->addWidget(m_indicatorInfoStyleComboBox);
        m_indicatorInfoStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorInfoStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorInfoStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_INFO);
            m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorInfoColorButton->text()), INDICATOR_INFO);
            m_diagnosticPreviewEditor->fillIndicatorRange(2, 0, 2, m_diagnosticPreviewEditor->text(2).length(), INDICATOR_INFO);
        });
        auto *indicatorInfoColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorInfoColorWidget);
        auto *indicatorInfoColorLayout = new QHBoxLayout(indicatorInfoColorWidget); // NOLINT
        indicatorInfoColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorInfoColorLabel = new QLabel(tr("Info Color")); // NOLINT
        indicatorInfoColorLayout->addWidget(indicatorInfoColorLabel, 1);
        indicatorInfoColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorInfoColorLayout->addWidget(m_indicatorInfoColorButton, 1);
        m_indicatorInfoColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorInfoColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorInfoColorButton->text(), this, tr("Choose Info Background")); newColor.isValid()) {
                m_indicatorInfoColorButton->setText(newColor.name());
                m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_INFO);
                m_diagnosticPreviewEditor->fillIndicatorRange(2, 0, 2, m_diagnosticPreviewEditor->text(2).length(), INDICATOR_INFO);
            }
        });

        auto *indicatorHintStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHintStyleWidget);
        auto *indicatorHintStyleLayout = new QHBoxLayout(indicatorHintStyleWidget); // NOLINT
        indicatorHintStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHintStyleLabel = new QLabel(tr("Hint Style")); // NOLINT
        indicatorHintStyleLayout->addWidget(indicatorHintStyleLabel);
        indicatorHintStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorHintStyleLayout->addWidget(m_indicatorHintStyleComboBox);
        m_indicatorHintStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorHintStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHintStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_HINT);
            m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorHintColorButton->text()), INDICATOR_HINT);
            m_diagnosticPreviewEditor->fillIndicatorRange(3, 0, 3, m_diagnosticPreviewEditor->text(3).length(), INDICATOR_HINT);
        });
        auto *indicatorHintColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHintColorWidget);
        auto *indicatorHintColorLayout = new QHBoxLayout(indicatorHintColorWidget); // NOLINT
        indicatorHintColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHintColorLabel = new QLabel(tr("Hint Color")); // NOLINT
        indicatorHintColorLayout->addWidget(indicatorHintColorLabel, 1);
        indicatorHintColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorHintColorLayout->addWidget(m_indicatorHintColorButton, 1);
        m_indicatorHintColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHintColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorHintColorButton->text(), this, tr("Choose Hint Background")); newColor.isValid()) {
                m_indicatorHintColorButton->setText(newColor.name());
                m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_HINT);
                m_diagnosticPreviewEditor->fillIndicatorRange(3, 0, 3, m_diagnosticPreviewEditor->text(3).length(), INDICATOR_HINT);
            }
        });

        layout->addWidget(m_diagnosticPreviewEditor);
        m_diagnosticPreviewEditor->setFixedHeight(200);
        m_diagnosticPreviewEditor->setFont(QFont("Consolas", 12));
        m_diagnosticPreviewEditor->setMarginType(0, QsciScintilla::NumberMargin);
        m_diagnosticPreviewEditor->setMarginWidth(0, 16);
        m_diagnosticPreviewEditor->setMarginWidth(1, 0);
        m_diagnosticPreviewEditor->setReadOnly(true);
    }
    // highlight
    {
        auto *highlightLabel = new QLabel(tr("Highlight")); // NOLINT
        layout->addWidget(highlightLabel);
        highlightLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(HLine);
        horizontalLine->setLineWidth(3);

        auto *indicatorHighlightStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHighlightStyleWidget);
        auto *indicatorHighlightStyleLayout = new QHBoxLayout(indicatorHighlightStyleWidget); // NOLINT
        indicatorHighlightStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHighlightStyleLabel = new QLabel(tr("Highlight Style")); // NOLINT
        indicatorHighlightStyleLayout->addWidget(indicatorHighlightStyleLabel);
        indicatorHighlightStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorHighlightStyleLayout->addWidget(m_indicatorHighlightStyleComboBox);
        m_indicatorHighlightStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorHighlightStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHighlightStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorHighlightColorButton->text()), INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_HIGHLIGHT);
            m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_HIGHLIGHT);
        });
        auto *indicatorHighlightColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHighlightColorWidget);
        auto *indicatorHighlightColorLayout = new QHBoxLayout(indicatorHighlightColorWidget); // NOLINT
        indicatorHighlightColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHighlightColorLabel = new QLabel(tr("Highlight Color")); // NOLINT
        indicatorHighlightColorLayout->addWidget(indicatorHighlightColorLabel, 1);
        indicatorHighlightColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorHighlightColorLayout->addWidget(m_indicatorHighlightColorButton, 1);
        m_indicatorHighlightColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHighlightColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorHighlightColorButton->text(), this, tr("Choose Highlight Result Background")); newColor.isValid()) {
                m_indicatorHighlightColorButton->setText(newColor.name());
                m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_HIGHLIGHT);
                m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_HIGHLIGHT);
                m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_HIGHLIGHT);
                m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_HIGHLIGHT);
                m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_HIGHLIGHT);
                m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_HIGHLIGHT);
            }
        });

        auto *indicatorReadStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorReadStyleWidget);
        auto *indicatorReadStyleLayout = new QHBoxLayout(indicatorReadStyleWidget); // NOLINT
        indicatorReadStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorReadStyleLabel = new QLabel(tr("Read Style")); // NOLINT
        indicatorReadStyleLayout->addWidget(indicatorReadStyleLabel);
        indicatorReadStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorReadStyleLayout->addWidget(m_indicatorReadStyleComboBox);
        m_indicatorReadStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorReadStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorReadStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_READ);
            m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorReadColorButton->text()), INDICATOR_READ);
            m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_READ);
            m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_READ);
            m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_READ);
        });
        auto *indicatorReadColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorReadColorWidget);
        auto *indicatorReadColorLayout = new QHBoxLayout(indicatorReadColorWidget); // NOLINT
        indicatorReadColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorReadColorLabel = new QLabel(tr("Read Color")); // NOLINT
        indicatorReadColorLayout->addWidget(indicatorReadColorLabel, 1);
        indicatorReadColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorReadColorLayout->addWidget(m_indicatorReadColorButton, 1);
        m_indicatorReadColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorReadColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorReadColorButton->text(), this, tr("Choose Read Result Background")); newColor.isValid()) {
                m_indicatorReadColorButton->setText(newColor.name());
                m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_READ);
                m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_READ);
                m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_READ);
                m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_READ);
            }
        });

        auto *indicatorWriteStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorWriteStyleWidget);
        auto *indicatorWriteStyleLayout = new QHBoxLayout(indicatorWriteStyleWidget); // NOLINT
        indicatorWriteStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorWriteStyleLabel = new QLabel(tr("Write Style")); // NOLINT
        indicatorWriteStyleLayout->addWidget(indicatorWriteStyleLabel);
        indicatorWriteStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorWriteStyleLayout->addWidget(m_indicatorWriteStyleComboBox);
        m_indicatorWriteStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorWriteStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorWriteStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_WRITE);
            m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorWriteColorButton->text()), INDICATOR_WRITE);
            m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_WRITE);
            m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_WRITE);
        });
        auto *indicatorWriteColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorWriteColorWidget);
        auto *indicatorWriteColorLayout = new QHBoxLayout(indicatorWriteColorWidget); // NOLINT
        indicatorWriteColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorWriteColorLabel = new QLabel(tr("Write Color")); // NOLINT
        indicatorWriteColorLayout->addWidget(indicatorWriteColorLabel, 1);
        indicatorWriteColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorWriteColorLayout->addWidget(m_indicatorWriteColorButton, 1);
        m_indicatorWriteColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorWriteColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorWriteColorButton->text(), this, tr("Choose Write Result Background")); newColor.isValid()) {
                m_indicatorWriteColorButton->setText(newColor.name());
                m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_WRITE);
                m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_WRITE);
                m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_WRITE);
            }
        });

        layout->addWidget(m_highlightPreviewEditor);
        m_highlightPreviewEditor->setFixedHeight(200);
        m_highlightPreviewEditor->setFont(QFont("Consolas", 12));
        m_highlightPreviewEditor->setMarginType(0, QsciScintilla::NumberMargin);
        m_highlightPreviewEditor->setMarginWidth(0, 16);
        m_highlightPreviewEditor->setMarginWidth(1, 0);
        m_highlightPreviewEditor->setReadOnly(true);
    }
    // search
    {
        auto *searchLabel = new QLabel(tr("Search")); // NOLINT
        layout->addWidget(searchLabel);
        searchLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(HLine);
        horizontalLine->setLineWidth(3);

        auto *indicatorSearchStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchStyleWidget);
        auto *indicatorSearchStyleLayout = new QHBoxLayout(indicatorSearchStyleWidget); // NOLINT
        indicatorSearchStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchStyleLabel = new QLabel(tr("Search Style")); // NOLINT
        indicatorSearchStyleLayout->addWidget(indicatorSearchStyleLabel);
        indicatorSearchStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchStyleLayout->addWidget(m_indicatorSearchStyleComboBox);
        m_indicatorSearchStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorSearchStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_SEARCH);
            m_searchPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorSearchColorButton->text()), INDICATOR_SEARCH);
            m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH);
            m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH);
            m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH);
            m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH);
            m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH);
        });
        auto *indicatorSearchColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchColorWidget);
        auto *indicatorSearchColorLayout = new QHBoxLayout(indicatorSearchColorWidget); // NOLINT
        indicatorSearchColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchColorLabel = new QLabel(tr("Search Color")); // NOLINT
        indicatorSearchColorLayout->addWidget(indicatorSearchColorLabel, 1);
        indicatorSearchColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchColorLayout->addWidget(m_indicatorSearchColorButton, 1);
        m_indicatorSearchColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorSearchColorButton->text(), this, tr("Choose Search Result Background")); newColor.isValid()) {
                m_indicatorSearchColorButton->setText(newColor.name());
                m_searchPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_SEARCH);
                m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH);
                m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH);
                m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH);
                m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH);
                m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH);
            }
        });

        auto *indicatorSelectionStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSelectionStyleWidget);
        auto *indicatorSelectionStyleLayout = new QHBoxLayout(indicatorSelectionStyleWidget); // NOLINT
        indicatorSelectionStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSelectionStyleLabel = new QLabel(tr("Selection Style")); // NOLINT
        indicatorSelectionStyleLayout->addWidget(indicatorSelectionStyleLabel);
        indicatorSelectionStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorSelectionStyleLayout->addWidget(m_indicatorSelectionStyleComboBox);
        m_indicatorSelectionStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorSelectionStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSelectionStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_SELECTION);
            m_searchPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorSelectionColorButton->text()), INDICATOR_SELECTION);
            m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SELECTION);
        });
        auto *indicatorSelectionColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSelectionColorWidget);
        auto *indicatorSelectionColorLayout = new QHBoxLayout(indicatorSelectionColorWidget); // NOLINT
        indicatorSelectionColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSelectionColorLabel = new QLabel(tr("Selection Color")); // NOLINT
        indicatorSelectionColorLayout->addWidget(indicatorSelectionColorLabel, 1);
        indicatorSelectionColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorSelectionColorLayout->addWidget(m_indicatorSelectionColorButton, 1);
        m_indicatorSelectionColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSelectionColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorSelectionColorButton->text(), this, tr("Choose Search Current Background")); newColor.isValid()) {
                m_indicatorSelectionColorButton->setText(newColor.name());
                m_searchPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_SELECTION);
                m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SELECTION);
            }
        });

        layout->addWidget(m_searchPreviewEditor);
        m_searchPreviewEditor->setFixedHeight(200);
        m_searchPreviewEditor->setFont(QFont("Consolas", 12));
        m_searchPreviewEditor->setMarginType(0, QsciScintilla::NumberMargin);
        m_searchPreviewEditor->setMarginWidth(0, 16);
        m_searchPreviewEditor->setMarginWidth(1, 0);
        m_searchPreviewEditor->setReadOnly(true);
    }
    // hyperlink
    {
        auto *hyperlinkLabel = new QLabel(tr("Hyperlink")); // NOLINT
        layout->addWidget(hyperlinkLabel);
        hyperlinkLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(HLine);
        horizontalLine->setLineWidth(3);

        auto *indicatorHyperlinkStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHyperlinkStyleWidget);
        auto *indicatorHyperlinkStyleLayout = new QHBoxLayout(indicatorHyperlinkStyleWidget); // NOLINT
        indicatorHyperlinkStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHyperlinkStyleLabel = new QLabel(tr("Hyperlink Style")); // NOLINT
        indicatorHyperlinkStyleLayout->addWidget(indicatorHyperlinkStyleLabel);
        indicatorHyperlinkStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorHyperlinkStyleLayout->addWidget(m_indicatorHyperlinkStyleComboBox);
        m_indicatorHyperlinkStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorHyperlinkStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHyperlinkStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_hyperlinkPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_HYPERLINK);
            m_hyperlinkPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorHyperlinkColorButton->text()), INDICATOR_HYPERLINK);
            m_hyperlinkPreviewEditor->fillIndicatorRange(0, 9, 0, 15, INDICATOR_HYPERLINK);
        });
        auto *indicatorHyperlinkColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorHyperlinkColorWidget);
        auto *indicatorHyperlinkColorLayout = new QHBoxLayout(indicatorHyperlinkColorWidget); // NOLINT
        indicatorHyperlinkColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorHyperlinkColorLabel = new QLabel(tr("Hyperlink Color")); // NOLINT
        indicatorHyperlinkColorLayout->addWidget(indicatorHyperlinkColorLabel, 1);
        indicatorHyperlinkColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorHyperlinkColorLayout->addWidget(m_indicatorHyperlinkColorButton, 1);
        m_indicatorHyperlinkColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorHyperlinkColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorHyperlinkColorButton->text(), this, tr("Choose Hyperlink Result Background")); newColor.isValid()) {
                m_indicatorHyperlinkColorButton->setText(newColor.name());
                m_hyperlinkPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_HYPERLINK);
                m_hyperlinkPreviewEditor->fillIndicatorRange(0, 9, 0, 15, INDICATOR_HYPERLINK);
            }
        });

        layout->addWidget(m_hyperlinkPreviewEditor);
        m_hyperlinkPreviewEditor->setFixedHeight(200);
        m_hyperlinkPreviewEditor->setFont(QFont("Consolas", 12));
        m_hyperlinkPreviewEditor->setMarginType(0, QsciScintilla::NumberMargin);
        m_hyperlinkPreviewEditor->setMarginWidth(0, 16);
        m_hyperlinkPreviewEditor->setMarginWidth(1, 0);
        m_hyperlinkPreviewEditor->setReadOnly(true);
    }
}

void IndicatorSettingScript::settingImport(const QJsonObject &indicatorConfigScript) const {
    // diagnostic
    {
        const int indicatorErrorStyle = indicatorConfigScript["indicatorErrorStyle"].toInt();
        m_indicatorErrorStyleComboBox->setCurrentIndex(indicatorErrorStyle);
        const QString indicatorErrorColor = indicatorConfigScript["indicatorErrorColor"].toString();
        m_indicatorErrorColorButton->setText(indicatorErrorColor);
        const int indicatorWarningStyle = indicatorConfigScript["indicatorWarningStyle"].toInt();
        m_indicatorWarningStyleComboBox->setCurrentIndex(indicatorWarningStyle);
        const QString indicatorWarningColor = indicatorConfigScript["indicatorWarningColor"].toString();
        m_indicatorWarningColorButton->setText(indicatorWarningColor);
        const int indicatorInfoStyle = indicatorConfigScript["indicatorInfoStyle"].toInt();
        m_indicatorInfoStyleComboBox->setCurrentIndex(indicatorInfoStyle);
        const QString indicatorInfoColor = indicatorConfigScript["indicatorInfoColor"].toString();
        m_indicatorInfoColorButton->setText(indicatorInfoColor);
        const int indicatorHintStyle = indicatorConfigScript["indicatorHintStyle"].toInt();
        m_indicatorHintStyleComboBox->setCurrentIndex(indicatorHintStyle);
        const QString indicatorHintColor = indicatorConfigScript["indicatorHintColor"].toString();
        m_indicatorHintColorButton->setText(indicatorHintColor);

        m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorErrorStyle), INDICATOR_ERROR);
        m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(indicatorErrorColor), INDICATOR_ERROR);
        m_diagnosticPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_ERROR);
        m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorWarningStyle), INDICATOR_WARNING);
        m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(indicatorWarningColor), INDICATOR_WARNING);
        m_diagnosticPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_WARNING);
        m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorInfoStyle), INDICATOR_INFO);
        m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(indicatorInfoColor), INDICATOR_INFO);
        m_diagnosticPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_INFO);
        m_diagnosticPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorHintStyle), INDICATOR_HINT);
        m_diagnosticPreviewEditor->setIndicatorForegroundColor(QColor(indicatorHintColor), INDICATOR_HINT);
        m_diagnosticPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_HINT);
        m_diagnosticPreviewEditor->setText(tr(
            "Error: A serious issue that is likely to prevent the code from working correctly.\n"
            "Warning: A less serious issue that may lead to bugs, or indicates suspicious code, but the code still \"works\".\n"
            "Info: Helpful information about the code - not necessarily a bug, more like a note or suggestion.\n"
            "Hint: The lowest severity; typically stylistic suggestions, minor improvements, or optional enhancements."));
        m_diagnosticPreviewEditor->fillIndicatorRange(0, 0, 0, m_diagnosticPreviewEditor->text(0).length(), INDICATOR_ERROR);
        m_diagnosticPreviewEditor->fillIndicatorRange(1, 0, 1, m_diagnosticPreviewEditor->text(1).length(), INDICATOR_WARNING);
        m_diagnosticPreviewEditor->fillIndicatorRange(2, 0, 2, m_diagnosticPreviewEditor->text(2).length(), INDICATOR_INFO);
        m_diagnosticPreviewEditor->fillIndicatorRange(3, 0, 3, m_diagnosticPreviewEditor->text(3).length(), INDICATOR_HINT);
    }
    // highlight
    {
        const int indicatorHighlightStyle = indicatorConfigScript["indicatorHighlightStyle"].toInt();
        m_indicatorHighlightStyleComboBox->setCurrentIndex(indicatorHighlightStyle);
        const QString indicatorHighlightColor = indicatorConfigScript["indicatorHighlightColor"].toString();
        m_indicatorHighlightColorButton->setText(indicatorHighlightColor);
        const int indicatorReadStyle = indicatorConfigScript["indicatorReadStyle"].toInt();
        m_indicatorReadStyleComboBox->setCurrentIndex(indicatorReadStyle);
        const QString indicatorReadColor = indicatorConfigScript["indicatorReadColor"].toString();
        m_indicatorReadColorButton->setText(indicatorReadColor);
        const int indicatorWriteStyle = indicatorConfigScript["indicatorWriteStyle"].toInt();
        m_indicatorWriteStyleComboBox->setCurrentIndex(indicatorWriteStyle);
        const QString indicatorWriteColor = indicatorConfigScript["indicatorWriteColor"].toString();
        m_indicatorWriteColorButton->setText(indicatorWriteColor);

        m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorHighlightStyle), INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(indicatorHighlightColor), INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorReadStyle), INDICATOR_READ);
        m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(indicatorReadColor), INDICATOR_READ);
        m_highlightPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_READ);
        m_highlightPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorWriteStyle), INDICATOR_WRITE);
        m_highlightPreviewEditor->setIndicatorForegroundColor(QColor(indicatorWriteColor), INDICATOR_WRITE);
        m_highlightPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_WRITE);
        m_highlightPreviewEditor->setText(
            "local count = 0\n"
            "while count < 10 do\n"
            "    print(count)\n"
            "    count = count + 1\n"
            "end");
        m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_HIGHLIGHT);
        m_highlightPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_READ);
        m_highlightPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_READ);
        m_highlightPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_READ);
        m_highlightPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_WRITE);
        m_highlightPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_WRITE);
    }
    // search
    {
        const int indicatorSearchStyle = indicatorConfigScript["indicatorSearchStyle"].toInt();
        m_indicatorSearchStyleComboBox->setCurrentIndex(indicatorSearchStyle);
        const QString indicatorSearchColor = indicatorConfigScript["indicatorSearchColor"].toString();
        m_indicatorSearchColorButton->setText(indicatorSearchColor);
        const int indicatorSelectionStyle = indicatorConfigScript["indicatorSelectionStyle"].toInt();
        m_indicatorSelectionStyleComboBox->setCurrentIndex(indicatorSelectionStyle);
        const QString indicatorSelectionColor = indicatorConfigScript["indicatorSelectionColor"].toString();
        m_indicatorSelectionColorButton->setText(indicatorSelectionColor);

        m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorSearchStyle), INDICATOR_SEARCH);
        m_searchPreviewEditor->setIndicatorForegroundColor(QColor(indicatorSearchColor), INDICATOR_SEARCH);
        m_searchPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_SEARCH);
        m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorSelectionStyle), INDICATOR_SELECTION);
        m_searchPreviewEditor->setIndicatorForegroundColor(QColor(indicatorSelectionColor), INDICATOR_SELECTION);
        m_searchPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_SELECTION);
        m_searchPreviewEditor->setText(
            "local count = 0\n"
            "while count < 10 do\n"
            "    print(count)\n"
            "    count = count + 1\n"
            "end");
        m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH);
        m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH);
        m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH);
        m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH);
        m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH);
        m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SELECTION);
    }
    // hyperlink
    {
        const int indicatorHyperlinkStyle = indicatorConfigScript["indicatorHyperlinkStyle"].toInt();
        m_indicatorHyperlinkStyleComboBox->setCurrentIndex(indicatorHyperlinkStyle);
        const QString indicatorHyperlinkColor = indicatorConfigScript["indicatorHyperlinkColor"].toString();
        m_indicatorHyperlinkColorButton->setText(indicatorHyperlinkColor);

        m_hyperlinkPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorHyperlinkStyle), INDICATOR_HYPERLINK);
        m_hyperlinkPreviewEditor->setIndicatorForegroundColor(QColor(indicatorHyperlinkColor), INDICATOR_HYPERLINK);
        m_hyperlinkPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_HYPERLINK);
        m_hyperlinkPreviewEditor->setText(
            "function target()\n"
            "end");
        m_hyperlinkPreviewEditor->fillIndicatorRange(0, 9, 0, 15, INDICATOR_HYPERLINK);
    }
}

QJsonObject IndicatorSettingScript::settingExport() const {
    QJsonObject indicatorConfigScript = {};
    indicatorConfigScript["indicatorErrorStyle"] = m_indicatorErrorStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorErrorColor"] = m_indicatorErrorColorButton->text();
    indicatorConfigScript["indicatorWarningStyle"] = m_indicatorWarningStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorWarningColor"] = m_indicatorWarningColorButton->text();
    indicatorConfigScript["indicatorInfoStyle"] = m_indicatorInfoStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorInfoColor"] = m_indicatorInfoColorButton->text();
    indicatorConfigScript["indicatorHintStyle"] = m_indicatorHintStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorHintColor"] = m_indicatorHintColorButton->text();
    indicatorConfigScript["indicatorHighlightStyle"] = m_indicatorHighlightStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorHighlightColor"] = m_indicatorHighlightColorButton->text();
    indicatorConfigScript["indicatorReadStyle"] = m_indicatorReadStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorReadColor"] = m_indicatorReadColorButton->text();
    indicatorConfigScript["indicatorWriteStyle"] = m_indicatorWriteStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorWriteColor"] = m_indicatorWriteColorButton->text();
    indicatorConfigScript["indicatorSearchStyle"] = m_indicatorSearchStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorSearchColor"] = m_indicatorSearchColorButton->text();
    indicatorConfigScript["indicatorSelectionStyle"] = m_indicatorSelectionStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorSelectionColor"] = m_indicatorSelectionColorButton->text();
    indicatorConfigScript["indicatorHyperlinkStyle"] = m_indicatorHyperlinkStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorHyperlinkColor"] = m_indicatorHyperlinkColorButton->text();
    return indicatorConfigScript;
}
