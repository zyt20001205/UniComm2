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
      m_markerBreakpointColorButton(new QPushButton()),
      m_markerDebugStyleComboBox(new QComboBox()),
      m_markerDebugColorButton(new QPushButton()),
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
        m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerBreakpointColorButton->text()), MARKER_BREAKPOINT);
        m_markerPreviewEditor->markerAdd(0, MARKER_BREAKPOINT);
    });
    auto *markerBreakpointColorWidget = new QWidget(); // NOLINT
    layout->addWidget(markerBreakpointColorWidget);
    auto *markerBreakpointColorLayout = new QHBoxLayout(markerBreakpointColorWidget); // NOLINT
    markerBreakpointColorLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerBreakpointColorLabel = new QLabel(tr("Breakpoint Color")); // NOLINT
    markerBreakpointColorLayout->addWidget(markerBreakpointColorLabel, 1);
    markerBreakpointColorLabel->setFont(QFont("Segoe UI", 12));
    markerBreakpointColorLayout->addWidget(m_markerBreakpointColorButton, 1);
    m_markerBreakpointColorButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerBreakpointColorButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerBreakpointColorButton->text(), this, tr("Choose Breakpoint Background Color")); newColor.isValid()) {
            m_markerBreakpointColorButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerBreakpointColorButton->text()), MARKER_BREAKPOINT);
            m_markerPreviewEditor->markerAdd(0, MARKER_BREAKPOINT);
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
        m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerDebugColorButton->text()), MARKER_DEBUG);
        m_markerPreviewEditor->markerAdd(1, MARKER_DEBUG);
    });
    auto *markerDebugColorWidget = new QWidget(); // NOLINT
    layout->addWidget(markerDebugColorWidget);
    auto *markerDebugColorLayout = new QHBoxLayout(markerDebugColorWidget); // NOLINT
    markerDebugColorLayout->setContentsMargins(0, 0, 0, 0);
    auto *markerDebugColorLabel = new QLabel(tr("Debug Color")); // NOLINT
    markerDebugColorLayout->addWidget(markerDebugColorLabel, 1);
    markerDebugColorLabel->setFont(QFont("Segoe UI", 12));
    markerDebugColorLayout->addWidget(m_markerDebugColorButton, 1);
    m_markerDebugColorButton->setFont(QFont("Segoe UI", 12));
    connect(m_markerDebugColorButton, &QPushButton::clicked, this, [this] {
        if (const QColor newColor = QColorDialog::getColor(m_markerDebugColorButton->text(), this, tr("Choose Debug Background Color")); newColor.isValid()) {
            m_markerDebugColorButton->setText(newColor.name());
            m_markerPreviewEditor->setMarkerBackgroundColor(QColor(m_markerDebugColorButton->text()), MARKER_DEBUG);
            m_markerPreviewEditor->markerAdd(1, MARKER_DEBUG);
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
    const QString markerBreakpointColor = markerConfigScript["markerBreakpointColor"].toString();
    m_markerBreakpointColorButton->setText(markerBreakpointColor);
    const int markerDebugStyle = markerConfigScript["markerDebugStyle"].toInt();
    m_markerDebugStyleComboBox->setCurrentIndex(markerDebugStyle);
    const QString markerDebugColor = markerConfigScript["markerDebugColor"].toString();
    m_markerDebugColorButton->setText(markerDebugColor);

    m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerBreakpointStyle"].toInt()), MARKER_BREAKPOINT);
    m_markerPreviewEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerBreakpointColor"].toString()), MARKER_BREAKPOINT);
    m_markerPreviewEditor->markerDefine(static_cast<QsciScintilla::MarkerSymbol>(markerConfigScript["markerDebugStyle"].toInt()), MARKER_DEBUG);
    m_markerPreviewEditor->setMarkerBackgroundColor(QColor(markerConfigScript["markerDebugColor"].toString()), MARKER_DEBUG);
    m_markerPreviewEditor->setText(
        "breakpoint marker\n"
        "debug marker\n");
    m_markerPreviewEditor->markerAdd(0, MARKER_BREAKPOINT);
    m_markerPreviewEditor->markerAdd(1, MARKER_DEBUG);
}

QJsonObject MarkerSettingScript::settingExport() const {
    QJsonObject markerConfigScript = {};
    markerConfigScript["markerBreakpointStyle"] = m_markerBreakpointStyleComboBox->currentIndex();
    markerConfigScript["markerBreakpointColor"] = m_markerBreakpointColorButton->text();
    markerConfigScript["markerDebugStyle"] = m_markerDebugStyleComboBox->currentIndex();
    markerConfigScript["markerDebugColor"] = m_markerDebugColorButton->text();
    return markerConfigScript;
}
