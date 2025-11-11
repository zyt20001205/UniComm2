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
          tr("Plain"), tr("Squiggle"), tr("TT"), tr("Diagonal"), tr("Strike"), tr("Hidden"), tr("Box"), tr("Round Box"),
          tr("Straight Box"), tr("Full Box"), tr("Dashes"), tr("Dots"), tr("Squiggle Low"), tr("Dot Box"), tr("Squiggle Pixmap"), tr("Thick Composition"),
          tr("Thin Composition"), tr("Text Color"), tr("Triangle"), tr("Triangle Character"), tr("Gradient"), tr("Centre Gradient")
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
      m_indicatorSearchResultStyleComboBox(new QComboBox()),
      m_indicatorSearchResultColorButton(new QPushButton()),
      m_indicatorSearchCurrentStyleComboBox(new QComboBox()),
      m_indicatorSearchCurrentColorButton(new QPushButton()),
      m_searchPreviewEditor(new QsciScintilla()) {
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
        horizontalLine->setFrameShape(QFrame::HLine);
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
    // search
    {
        auto *searchLabel = new QLabel(tr("Search")); // NOLINT
        layout->addWidget(searchLabel);
        searchLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(QFrame::HLine);
        horizontalLine->setLineWidth(3);

        auto *indicatorSearchResultStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchResultStyleWidget);
        auto *indicatorSearchResultStyleLayout = new QHBoxLayout(indicatorSearchResultStyleWidget); // NOLINT
        indicatorSearchResultStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchResultStyleLabel = new QLabel(tr("Result Style")); // NOLINT
        indicatorSearchResultStyleLayout->addWidget(indicatorSearchResultStyleLabel);
        indicatorSearchResultStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchResultStyleLayout->addWidget(m_indicatorSearchResultStyleComboBox);
        m_indicatorSearchResultStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorSearchResultStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchResultStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorSearchResultColorButton->text()), INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH_RESULT);
            m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH_RESULT);
        });
        auto *indicatorSearchResultColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchResultColorWidget);
        auto *indicatorSearchResultColorLayout = new QHBoxLayout(indicatorSearchResultColorWidget); // NOLINT
        indicatorSearchResultColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchResultColorLabel = new QLabel(tr("Result Color")); // NOLINT
        indicatorSearchResultColorLayout->addWidget(indicatorSearchResultColorLabel, 1);
        indicatorSearchResultColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchResultColorLayout->addWidget(m_indicatorSearchResultColorButton, 1);
        m_indicatorSearchResultColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchResultColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorSearchResultColorButton->text(), this, tr("Choose Search Result Background")); newColor.isValid()) {
                m_indicatorSearchResultColorButton->setText(newColor.name());
                m_searchPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_SEARCH_RESULT);
                m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH_RESULT);
                m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH_RESULT);
                m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_RESULT);
                m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH_RESULT);
                m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH_RESULT);
            }
        });

        auto *indicatorSearchCurrentStyleWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchCurrentStyleWidget);
        auto *indicatorSearchCurrentStyleLayout = new QHBoxLayout(indicatorSearchCurrentStyleWidget); // NOLINT
        indicatorSearchCurrentStyleLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchCurrentStyleLabel = new QLabel(tr("Current Style")); // NOLINT
        indicatorSearchCurrentStyleLayout->addWidget(indicatorSearchCurrentStyleLabel);
        indicatorSearchCurrentStyleLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchCurrentStyleLayout->addWidget(m_indicatorSearchCurrentStyleComboBox);
        m_indicatorSearchCurrentStyleComboBox->addItems(m_indicatorStyleList);
        m_indicatorSearchCurrentStyleComboBox->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchCurrentStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
            m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(style), INDICATOR_SEARCH_CURRENT);
            m_searchPreviewEditor->setIndicatorForegroundColor(QColor(m_indicatorSearchCurrentColorButton->text()), INDICATOR_SEARCH_CURRENT);
            m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_CURRENT);
        });
        auto *indicatorSearchCurrentColorWidget = new QWidget(); // NOLINT
        layout->addWidget(indicatorSearchCurrentColorWidget);
        auto *indicatorSearchCurrentColorLayout = new QHBoxLayout(indicatorSearchCurrentColorWidget); // NOLINT
        indicatorSearchCurrentColorLayout->setContentsMargins(0, 0, 0, 0);
        auto *indicatorSearchCurrentColorLabel = new QLabel(tr("Current Color")); // NOLINT
        indicatorSearchCurrentColorLayout->addWidget(indicatorSearchCurrentColorLabel, 1);
        indicatorSearchCurrentColorLabel->setFont(QFont("Segoe UI", 12));
        indicatorSearchCurrentColorLayout->addWidget(m_indicatorSearchCurrentColorButton, 1);
        m_indicatorSearchCurrentColorButton->setFont(QFont("Segoe UI", 12));
        connect(m_indicatorSearchCurrentColorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_indicatorSearchCurrentColorButton->text(), this, tr("Choose Search Current Background")); newColor.isValid()) {
                m_indicatorSearchCurrentColorButton->setText(newColor.name());
                m_searchPreviewEditor->setIndicatorForegroundColor(QColor(newColor), INDICATOR_SEARCH_CURRENT);
                m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_CURRENT);
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
    // search
    {
        const int indicatorSearchResultStyle = indicatorConfigScript["indicatorSearchResultStyle"].toInt();
        m_indicatorSearchResultStyleComboBox->setCurrentIndex(indicatorSearchResultStyle);
        const QString indicatorSearchResultColor = indicatorConfigScript["indicatorSearchResultColor"].toString();
        m_indicatorSearchResultColorButton->setText(indicatorSearchResultColor);
        const int indicatorSearchCurrentStyle = indicatorConfigScript["indicatorSearchCurrentStyle"].toInt();
        m_indicatorSearchCurrentStyleComboBox->setCurrentIndex(indicatorSearchCurrentStyle);
        const QString indicatorSearchCurrentColor = indicatorConfigScript["indicatorSearchCurrentColor"].toString();
        m_indicatorSearchCurrentColorButton->setText(indicatorSearchCurrentColor);

        m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorSearchResultStyle), INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->setIndicatorForegroundColor(QColor(indicatorSearchResultColor), INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->indicatorDefine(static_cast<QsciScintilla::IndicatorStyle>(indicatorSearchCurrentStyle), INDICATOR_SEARCH_CURRENT);
        m_searchPreviewEditor->setIndicatorForegroundColor(QColor(indicatorSearchCurrentColor), INDICATOR_SEARCH_CURRENT);
        m_searchPreviewEditor->setIndicatorDrawUnder(true, INDICATOR_SEARCH_CURRENT);
        m_searchPreviewEditor->setText(
            "local count = 0\n"
            "while count < 10 do\n"
            "    print(count)\n"
            "    count = count + 1\n"
            "end");
        m_searchPreviewEditor->fillIndicatorRange(0, 6, 0, 11, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->fillIndicatorRange(1, 6, 1, 11, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->fillIndicatorRange(3, 4, 3, 9, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->fillIndicatorRange(3, 12, 3, 17, INDICATOR_SEARCH_RESULT);
        m_searchPreviewEditor->fillIndicatorRange(2, 10, 2, 15, INDICATOR_SEARCH_CURRENT);
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
    indicatorConfigScript["indicatorSearchResultStyle"] = m_indicatorSearchResultStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorSearchResultColor"] = m_indicatorSearchResultColorButton->text();
    indicatorConfigScript["indicatorSearchCurrentStyle"] = m_indicatorSearchCurrentStyleComboBox->currentIndex();
    indicatorConfigScript["indicatorSearchCurrentColor"] = m_indicatorSearchCurrentColorButton->text();
    return indicatorConfigScript;
}
