#include "settingModule/markerSettingScript.h"

#include <QColorDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "Qsci/qsciscintilla.h"

#include "globals.h"

// MarkerSettingScript public
MarkerSettingScript::MarkerSettingScript(QWidget *parent)
    : QScrollArea(parent),
      m_markerStyleList{
          tr("Circle"), tr("Rectangle"), tr("RightTriangle"), tr("SmallRectangle"), tr("RightArrow"), tr("Invisible"), tr("DownTriangle"), tr("Minus"), tr("Plus"),
          tr("VerticalLine"), tr("BottomLeftCorner"), tr("LeftSideSplitter"), tr("BoxedPlus"), tr("BoxedPlusConnected"), tr("BoxedMinus"), tr("BoxedMinusConnected"),
          tr("RoundedBottomLeftCorner"), tr("LeftSideRoundedSplitter"), tr("CircledPlus"), tr("CircledPlusConnected"), tr("CircledMinus"), tr("CircledMinusConnected"),
          tr("Background"), tr("ThreeDots"), tr("ThreeRightArrows"), tr("FullRectangle"), tr("LeftRectangle"), tr("Underline"), tr("Bookmark")
      },
      m_markerBreakpointStyleComboBox(new QComboBox()),
      m_markerBreakpointBackgroundButton(new QPushButton()),
      m_markerBreakpointForegroundButton(new QPushButton()),
      m_markerDebugStyleComboBox(new QComboBox()),
      m_markerDebugBackgroundButton(new QPushButton()),
      m_markerDebugForegroundButton(new QPushButton()),
      m_markerPreviewEditor(new QsciScintilla()) {
    auto *widget = new QWidget(); // NOLINT
    setWidget(widget);
    setWidgetResizable(true);
    auto *layout = new QVBoxLayout(widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    auto *markerBreakpointStyleWidget = new QWidget(); // NOLINT
    layout->addWidget(markerBreakpointStyleWidget);
    auto *markerBreakpointStyleLayout = new QHBoxLayout(markerBreakpointStyleWidget); // NOLINT
    markerBreakpointStyleLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerBreakpointStyleLabel = new QLabel(tr("Breakpoint Style")); // NOLINT
    markerBreakpointStyleLayout->addWidget(markerBreakpointStyleLabel);
    markerBreakpointStyleLabel->setFont(QFont("Segoe UI", 12));
    markerBreakpointStyleLayout->addWidget(m_markerBreakpointStyleComboBox);
    m_markerBreakpointStyleComboBox->addItems(m_markerStyleList);
    m_markerBreakpointStyleComboBox->setFont(QFont("Segoe UI", 12));
    connect(m_markerBreakpointStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
        m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(style), MARKER_BREAKPOINT);
        m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerBreakpointBackgroundButton->text()), MARKER_BREAKPOINT);
        m_markerPreviewEditor->recolor();
    });
    auto *markerBreakpointBackgroundWidget = new QWidget(); // NOLINT
    layout->addWidget(markerBreakpointBackgroundWidget);
    auto *markerBreakpointBackgroundLayout = new QHBoxLayout(markerBreakpointBackgroundWidget); // NOLINT
    markerBreakpointBackgroundLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerBreakpointBackgroundLabel = new QLabel(tr("Breakpoint Background")); // NOLINT
    markerBreakpointBackgroundLayout->addWidget(markerBreakpointBackgroundLabel, 1);
    markerBreakpointBackgroundLabel->setFont(QFont("Segoe UI", 12));
    markerBreakpointBackgroundLayout->addWidget(m_markerBreakpointBackgroundButton, 1);
    m_markerBreakpointBackgroundButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerBreakpointBackgroundButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerBreakpointBackgroundButton->text(), this, tr("Choose Breakpoint Background")); newColor.isValid()) {
            m_markerBreakpointBackgroundButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerBreakpointBackgroundButton->text()), MARKER_BREAKPOINT);
            m_markerPreviewEditor->recolor();
        }
    });
    auto *markerBreakpointForegroundWidget = new QWidget(); // NOLINT
    layout->addWidget(markerBreakpointForegroundWidget);
    auto *markerBreakpointForegroundLayout = new QHBoxLayout(markerBreakpointForegroundWidget); // NOLINT
    markerBreakpointForegroundLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerBreakpointForegroundLabel = new QLabel(tr("Breakpoint Foreground")); // NOLINT
    markerBreakpointForegroundLayout->addWidget(markerBreakpointForegroundLabel, 1);
    markerBreakpointForegroundLabel->setFont(QFont("Segoe UI", 12));
    markerBreakpointForegroundLayout->addWidget(m_markerBreakpointForegroundButton, 1);
    m_markerBreakpointForegroundButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerBreakpointForegroundButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerBreakpointForegroundButton->text(), this, tr("Choose Breakpoint Foreground")); newColor.isValid()) {
            m_markerBreakpointForegroundButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerForegroundColor(QColor(m_markerBreakpointForegroundButton->text()), MARKER_BREAKPOINT);
            m_markerPreviewEditor->recolor();
        }
    });

    auto *markerDebugStyleWidget = new QWidget(); // NOLINT
    layout->addWidget(markerDebugStyleWidget);
    auto *markerDebugStyleLayout = new QHBoxLayout(markerDebugStyleWidget); // NOLINT
    markerDebugStyleLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerDebugStyleLabel = new QLabel(tr("Debug Style")); // NOLINT
    markerDebugStyleLayout->addWidget(markerDebugStyleLabel);
    markerDebugStyleLabel->setFont(QFont("Segoe UI", 12));
    markerDebugStyleLayout->addWidget(m_markerDebugStyleComboBox);
    m_markerDebugStyleComboBox->addItems(m_markerStyleList);
    m_markerDebugStyleComboBox->setFont(QFont("Segoe UI", 12));
    connect(m_markerDebugStyleComboBox, &QComboBox::currentIndexChanged, this, [this](const int style) {
        m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(style), MARKER_DEBUG);
        m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerDebugBackgroundButton->text()), MARKER_DEBUG);
        m_markerPreviewEditor->recolor();
    });
    auto *markerDebugBackgroundWidget = new QWidget(); // NOLINT
    layout->addWidget(markerDebugBackgroundWidget);
    auto *markerDebugBackgroundLayout = new QHBoxLayout(markerDebugBackgroundWidget); // NOLINT
    markerDebugBackgroundLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerDebugBackgroundLabel = new QLabel(tr("Debug Background")); // NOLINT
    markerDebugBackgroundLayout->addWidget(markerDebugBackgroundLabel, 1);
    markerDebugBackgroundLabel->setFont(QFont("Segoe UI", 12));
    markerDebugBackgroundLayout->addWidget(m_markerDebugBackgroundButton, 1);
    m_markerDebugBackgroundButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerDebugBackgroundButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerDebugBackgroundButton->text(), this, tr("Choose Debug Background")); newColor.isValid()) {
            m_markerDebugBackgroundButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerDebugBackgroundButton->text()), MARKER_DEBUG);
            m_markerPreviewEditor->recolor();
        }
    });
    auto *markerDebugForegroundWidget = new QWidget(); // NOLINT
    layout->addWidget(markerDebugForegroundWidget);
    auto *markerDebugForegroundLayout = new QHBoxLayout(markerDebugForegroundWidget); // NOLINT
    markerDebugForegroundLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerDebugForegroundLabel = new QLabel(tr("Debug Foreground")); // NOLINT
    markerDebugForegroundLayout->addWidget(markerDebugForegroundLabel, 1);
    markerDebugForegroundLabel->setFont(QFont("Segoe UI", 12));
    markerDebugForegroundLayout->addWidget(m_markerDebugForegroundButton, 1);
    m_markerDebugForegroundButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerDebugForegroundButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerDebugForegroundButton->text(), this, tr("Choose Debug Foreground")); newColor.isValid()) {
            m_markerDebugForegroundButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerForegroundColor(QColor(m_markerDebugForegroundButton->text()), MARKER_DEBUG);
            m_markerPreviewEditor->recolor();
        }
    });

    layout->addWidget(m_markerPreviewEditor);
    m_markerPreviewEditor->setFixedHeight(200);
    m_markerPreviewEditor->setFont(QFont("Consolas", 12));
    m_markerPreviewEditor->setMarginType(0, QsciScintilla::NumberMargin);
    m_markerPreviewEditor->setMarginWidth(0, 16);
    m_markerPreviewEditor->setMarginType(1, QsciScintilla::SymbolMargin);
    m_markerPreviewEditor->setMarginSensitivity(1, true);
    m_markerPreviewEditor->setMarginWidth(1, 16);
    m_markerPreviewEditor->setReadOnly(true);

    layout->addStretch();
}

void MarkerSettingScript::settingImport(const QJsonObject &markerConfigScript) const {
    const int markerBreakpointStyle = markerConfigScript["markerBreakpointStyle"].toInt();
    m_markerBreakpointStyleComboBox->setCurrentIndex(markerBreakpointStyle);
    const QString markerBreakpointBackground = markerConfigScript["markerBreakpointBackground"].toString();
    m_markerBreakpointBackgroundButton->setText(markerBreakpointBackground);
    const QString markerBreakpointForeground = markerConfigScript["markerBreakpointForeground"].toString();
    m_markerBreakpointForegroundButton->setText(markerBreakpointForeground);
    const int markerDebugStyle = markerConfigScript["markerDebugStyle"].toInt();
    m_markerDebugStyleComboBox->setCurrentIndex(markerDebugStyle);
    const QString markerDebugBackground = markerConfigScript["markerDebugBackground"].toString();
    m_markerDebugBackgroundButton->setText(markerDebugBackground);
    const QString markerDebugForeground = markerConfigScript["markerDebugForeground"].toString();
    m_markerDebugForegroundButton->setText(markerDebugForeground);

    m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
    m_markerPreviewEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointBackground"].toString()), MARKER_BREAKPOINT);
    m_markerPreviewEditor->setMarkerForegroundColor(QColor(markerConfigScript["markerBreakpointForeground"].toString()), MARKER_BREAKPOINT);
    m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
    m_markerPreviewEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugBackground"].toString()), MARKER_DEBUG);
    m_markerPreviewEditor->setMarkerForegroundColor(QColor(markerConfigScript["markerDebugForeground"].toString()), MARKER_DEBUG);
    m_markerPreviewEditor->setText(
        "breakpoint marker\n"
        "debug marker\n");
    m_markerPreviewEditor->markerAdd(0, MARKER_BREAKPOINT);
    m_markerPreviewEditor->markerAdd(1, MARKER_DEBUG);
}

QJsonObject MarkerSettingScript::settingExport() const {
    QJsonObject markerConfigScript = {};
    markerConfigScript["markerBreakpointStyle"] = m_markerBreakpointStyleComboBox->currentIndex();
    markerConfigScript["markerBreakpointBackground"] = m_markerBreakpointBackgroundButton->text();
    markerConfigScript["markerBreakpointForeground"] = m_markerBreakpointForegroundButton->text();
    markerConfigScript["markerDebugStyle"] = m_markerDebugStyleComboBox->currentIndex();
    markerConfigScript["markerDebugBackground"] = m_markerDebugBackgroundButton->text();
    markerConfigScript["markerDebugForeground"] = m_markerDebugForegroundButton->text();
    return markerConfigScript;
}
