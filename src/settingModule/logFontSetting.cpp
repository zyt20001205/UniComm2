#include "settingModule/logFontSetting.h"

#include <QFontComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// LogFontSetting public
LogFontSetting::LogFontSetting(QWidget *parent)
    : QWidget(parent),
      m_fontFamilyComboBox(new QFontComboBox()),
      m_fontSizeSpinBox(new QSpinBox()),
      m_fontPreviewTextEdit(new QTextEdit()) {
    auto *layout = new QVBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);

    auto *fontFamilyWidget = new QWidget(); // NOLINT
    layout->addWidget(fontFamilyWidget);
    auto *fontFamilyLayout = new QHBoxLayout(fontFamilyWidget); // NOLINT
    fontFamilyLayout->setContentsMargins(0, 0, 0, 0);
    auto *fontFamilyLabel = new QLabel(tr("Font Family")); // NOLINT
    fontFamilyLayout->addWidget(fontFamilyLabel);
    fontFamilyLabel->setFont(QFont("Consolas", 12));
    fontFamilyLayout->addWidget(m_fontFamilyComboBox);
    m_fontFamilyComboBox->setFont(QFont("Consolas", 12));
    connect(m_fontFamilyComboBox, &QFontComboBox::currentTextChanged, this, [this](const QString &fontFamily) {
        QFont font = m_fontPreviewTextEdit->font();
        font.setFamily(fontFamily);
        m_fontPreviewTextEdit->setFont(font);
    });

    auto *fontSizeWidget = new QWidget(); // NOLINT
    layout->addWidget(fontSizeWidget);
    auto *fontSizeLayout = new QHBoxLayout(fontSizeWidget); // NOLINT
    fontSizeLayout->setContentsMargins(0, 0, 0, 0);
    auto *fontSizeLabel = new QLabel(tr("Font Size")); // NOLINT
    fontSizeLayout->addWidget(fontSizeLabel);
    fontSizeLabel->setFont(QFont("Consolas", 12));
    fontSizeLayout->addWidget(m_fontSizeSpinBox);
    m_fontSizeSpinBox->setFont(QFont("Consolas", 12));
    connect(m_fontSizeSpinBox, &QSpinBox::valueChanged, this, [this](const int &fontSize) {
        QFont font = m_fontPreviewTextEdit->font();
        font.setPointSize(fontSize);
        m_fontPreviewTextEdit->setFont(font);
    });

    layout->addWidget(m_fontPreviewTextEdit);
}

void LogFontSetting::settingImport(const QJsonObject &logFontConfig) const {
    m_fontFamilyComboBox->setCurrentText(logFontConfig["fontFamily"].toString());
    m_fontSizeSpinBox->setValue(logFontConfig["fontSize"].toInt());
    m_fontPreviewTextEdit->setText(
        "abcdefghijklmnopqrstuvwxyz\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        "0123456789 (){}[]\n"
        "+ - * / = .,;:!? #&$%@|^");
    m_fontPreviewTextEdit->setFont(QFont(logFontConfig["fontFamily"].toString(), logFontConfig["fontSize"].toInt()));
}

QJsonObject LogFontSetting::settingExport() const {
    QJsonObject logFontConfig = {};
    logFontConfig["fontFamily"] = m_fontFamilyComboBox->currentText();
    logFontConfig["fontSize"] = m_fontSizeSpinBox->value();
    return logFontConfig;
}
