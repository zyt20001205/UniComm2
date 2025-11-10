#include "settingModule/fontSettingScript.h"

#include <QFontComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// FontSettingScript public
FontSettingScript::FontSettingScript(QWidget *parent)
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
    fontFamilyLabel->setFont(QFont("Segoe UI", 12));
    fontFamilyLayout->addWidget(m_fontFamilyComboBox);
    m_fontFamilyComboBox->setFont(QFont("Segoe UI", 12));
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
    fontSizeLabel->setFont(QFont("Segoe UI", 12));
    fontSizeLayout->addWidget(m_fontSizeSpinBox);
    m_fontSizeSpinBox->setFont(QFont("Segoe UI", 12));
    connect(m_fontSizeSpinBox, &QSpinBox::valueChanged, this, [this](const int &fontSize) {
        QFont font = m_fontPreviewTextEdit->font();
        font.setPointSize(fontSize);
        m_fontPreviewTextEdit->setFont(font);
    });

    layout->addWidget(m_fontPreviewTextEdit);
}

void FontSettingScript::settingImport(const QJsonObject &fontConfigScript) const {
    m_fontFamilyComboBox->setCurrentText(fontConfigScript["fontFamily"].toString());
    m_fontSizeSpinBox->setValue(fontConfigScript["fontSize"].toInt());
    m_fontPreviewTextEdit->setText(
        "abcdefghijklmnopqrstuvwxyz\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        "0123456789 (){}[]\n"
        "+ - * / = .,;:!? #&$%@|^");
    m_fontPreviewTextEdit->setFont(QFont(fontConfigScript["fontFamily"].toString(), fontConfigScript["fontSize"].toInt()));
}

QJsonObject FontSettingScript::settingExport() const {
    QJsonObject fontConfigScript = {};
    fontConfigScript["fontFamily"] = m_fontFamilyComboBox->currentText();
    fontConfigScript["fontSize"] = m_fontSizeSpinBox->value();
    return fontConfigScript;
}
