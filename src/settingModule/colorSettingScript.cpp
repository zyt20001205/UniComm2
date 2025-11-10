#include "settingModule/colorSettingScript.h"

#include <qcolordialog.h>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// ColorSettingScript public
ColorSettingScript::ColorSettingScript(QWidget *parent)
    : QWidget(parent),
      m_backgroundErrorButton(new QPushButton()),
      m_backgroundWarningButton(new QPushButton()),
      m_backgroundInfoButton(new QPushButton()),
      m_backgroundHintButton(new QPushButton()),
      m_colorPreviewTextEdit(new QTextEdit()) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    // diagnostics
    {
        auto *diagnosticsLabel = new QLabel(tr("Diagnostics")); // NOLINT
        layout->addWidget(diagnosticsLabel);
        diagnosticsLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
        QFrame *horizontalLine = new QFrame(); // NOLINT
        layout->addWidget(horizontalLine);
        horizontalLine->setFrameShape(QFrame::HLine);
        horizontalLine->setLineWidth(3);

        auto *backgroundErrorWidget = new QWidget(); // NOLINT
        layout->addWidget(backgroundErrorWidget);
        auto *backgroundErrorLayout = new QHBoxLayout(backgroundErrorWidget); // NOLINT
        backgroundErrorLayout->setContentsMargins(0, 0, 0, 0);
        auto *backgroundErrorLabel = new QLabel(tr("Error")); // NOLINT
        backgroundErrorLayout->addWidget(backgroundErrorLabel, 1);
        backgroundErrorLabel->setFont(QFont("Segoe UI", 12));
        backgroundErrorLayout->addWidget(m_backgroundErrorButton, 1);
        m_backgroundErrorButton->setFont(QFont("Segoe UI", 12));
        connect(m_backgroundErrorButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_backgroundErrorButton->text(), this, tr("Choose Error Background")); newColor.isValid()) {
                m_backgroundErrorButton->setText(newColor.name());
            }
        });

        auto *backgroundWarningWidget = new QWidget(); // NOLINT
        layout->addWidget(backgroundWarningWidget);
        auto *backgroundWarningLayout = new QHBoxLayout(backgroundWarningWidget); // NOLINT
        backgroundWarningLayout->setContentsMargins(0, 0, 0, 0);
        auto *backgroundWarningLabel = new QLabel(tr("Warning")); // NOLINT
        backgroundWarningLayout->addWidget(backgroundWarningLabel, 1);
        backgroundWarningLabel->setFont(QFont("Segoe UI", 12));
        backgroundWarningLayout->addWidget(m_backgroundWarningButton, 1);
        m_backgroundWarningButton->setFont(QFont("Segoe UI", 12));
        connect(m_backgroundWarningButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_backgroundWarningButton->text(), this, tr("Choose Warning Background")); newColor.isValid()) {
                m_backgroundWarningButton->setText(newColor.name());
            }
        });

        auto *backgroundInfoWidget = new QWidget(); // NOLINT
        layout->addWidget(backgroundInfoWidget);
        auto *backgroundInfoLayout = new QHBoxLayout(backgroundInfoWidget); // NOLINT
        backgroundInfoLayout->setContentsMargins(0, 0, 0, 0);
        auto *backgroundInfoLabel = new QLabel(tr("Info")); // NOLINT
        backgroundInfoLayout->addWidget(backgroundInfoLabel, 1);
        backgroundInfoLabel->setFont(QFont("Segoe UI", 12));
        backgroundInfoLayout->addWidget(m_backgroundInfoButton, 1);
        m_backgroundInfoButton->setFont(QFont("Segoe UI", 12));
        connect(m_backgroundInfoButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_backgroundInfoButton->text(), this, tr("Choose Info Background")); newColor.isValid()) {
                m_backgroundInfoButton->setText(newColor.name());
            }
        });

        auto *backgroundHintWidget = new QWidget(); // NOLINT
        layout->addWidget(backgroundHintWidget);
        auto *backgroundHintLayout = new QHBoxLayout(backgroundHintWidget); // NOLINT
        backgroundHintLayout->setContentsMargins(0, 0, 0, 0);
        auto *backgroundHintLabel = new QLabel(tr("Hint")); // NOLINT
        backgroundHintLayout->addWidget(backgroundHintLabel, 1);
        backgroundHintLabel->setFont(QFont("Segoe UI", 12));
        backgroundHintLayout->addWidget(m_backgroundHintButton, 1);
        m_backgroundHintButton->setFont(QFont("Segoe UI", 12));
        connect(m_backgroundHintButton, &QPushButton::clicked, this, [this] {
            if (const QColor newColor = QColorDialog::getColor(m_backgroundHintButton->text(), this, tr("Choose Hint Background")); newColor.isValid()) {
                m_backgroundHintButton->setText(newColor.name());
            }
        });
        
        layout->addWidget(m_colorPreviewTextEdit);
    }
}

void ColorSettingScript::settingImport(const QJsonObject &colorConfigScript) const {
    m_backgroundErrorButton->setText(colorConfigScript["backgroundError"].toString());
    m_backgroundWarningButton->setText(colorConfigScript["backgroundWarning"].toString());
    m_backgroundInfoButton->setText(colorConfigScript["backgroundInfo"].toString());
    m_backgroundHintButton->setText(colorConfigScript["backgroundHint"].toString());
    // m_colorPreviewTextEdit->setText(
    //     "abcdefghijklmnopqrstuvwxyz\n"
    //     "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
    //     "0123456789 (){}[]\n"
    //     "+ - * / = .,;:!? #&$%@|^");
    // m_colorPreviewTextEdit->setColor(QColor(colorConfigScript["backgroundError"].toString(), colorConfigScript["colorSize"].toInt()));
}

QJsonObject ColorSettingScript::settingExport() const {
    QJsonObject colorConfigScript = {};
    colorConfigScript["backgroundError"] = m_backgroundErrorButton->text();
    return colorConfigScript;
}
