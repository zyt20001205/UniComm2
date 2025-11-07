#include "settingModule/settingModule.h"

#include <QHBoxLayout>
#include <QJsonObject>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTreeView>

#include "settingModule/fontSettingLog.h"
#include "settingModule/fontSettingScript.h"

// SettingModule public
SettingModule::SettingModule(QWidget *parent)
    : QDialog(parent),
      m_settingStackedWidget(new QStackedWidget()),
      m_fontSettingLogWidget(new FontSettingLog()),
      m_fontSettingScriptWidget(new FontSettingScript()) {
    auto *layout = new QVBoxLayout(this); //NOLINT
    auto *splitter = new QSplitter(); // NOLINT
    layout->addWidget(splitter);

    auto *settingTreeView = new QTreeView(); // NOLINT
    splitter->addWidget(settingTreeView);
    settingTreeView->setFont(QFont("Consolas", 14));
    settingTreeView->setHeaderHidden(true);
    auto *settingTreeModel = new QStandardItemModel(); // NOLINT
    settingTreeView->setModel(settingTreeModel);
    // font setting
    auto *fontSetting = new QStandardItem(tr("Font Setting")); // NOLINT
    settingTreeModel->appendRow(fontSetting);
    auto *fontSettingLog = new QStandardItem(tr("Log Module")); // NOLINT
    fontSetting->appendRow(fontSettingLog);
    fontSettingLog->setData(FONT_SETTING_LOG, Qt::UserRole + 1);
    auto *fontSettingScript = new QStandardItem(tr("Script Module")); // NOLINT
    fontSetting->appendRow(fontSettingScript);
    fontSettingScript->setData(FONT_SETTING_SCRIPT, Qt::UserRole + 1);

    connect(settingTreeView, &QTreeView::clicked, this, [this, settingTreeModel](const QModelIndex &index) {
        const QStandardItem *item = settingTreeModel->itemFromIndex(index);
        const int pageIndex = item->data(Qt::UserRole + 1).toInt();
        m_settingStackedWidget->setCurrentIndex(pageIndex);
    });

    splitter->addWidget(m_settingStackedWidget);
    m_settingStackedWidget->addWidget(new QWidget());
    m_settingStackedWidget->addWidget(m_fontSettingLogWidget);
    m_settingStackedWidget->addWidget(m_fontSettingScriptWidget);
    m_settingStackedWidget->setCurrentIndex(0);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    auto *controlWidget = new QWidget(); // NOLINT
    layout->addWidget(controlWidget);
    controlWidget->setFixedHeight(30);
    auto *controlLayout = new QHBoxLayout(controlWidget); // NOLINT
    controlLayout->setAlignment(Qt::AlignRight);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    auto *saveButton = new QPushButton(tr("Save")); // NOLINT
    controlLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, [this] { settingSave(); });
    auto *cancelButton = new QPushButton(tr("Cancel")); // NOLINT
    controlLayout->addWidget(cancelButton);
    auto *applyButton = new QPushButton(tr("Apply")); // NOLINT
    controlLayout->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &SettingModule::settingApply);

    resize(1280, 720);
}

void SettingModule::settingImport(const QJsonObject &settingConfig) const {
    QJsonObject fontConfigLog = {};
    fontConfigLog["fontFamily"] = settingConfig["fontFamilyLog"].toString();
    fontConfigLog["fontSize"] = settingConfig["fontSizeLog"].toInt();
    m_fontSettingLogWidget->settingImport(fontConfigLog);
    QJsonObject fontConfigScript = {};
    fontConfigScript["fontFamily"] = settingConfig["fontFamilyScript"].toString();
    fontConfigScript["fontSize"] = settingConfig["fontSizeScript"].toInt();
    m_fontSettingScriptWidget->settingImport(fontConfigScript);
}

// SettingModule private
void SettingModule::settingApply() {
    const QJsonObject fontConfigLog = m_fontSettingLogWidget->settingExport();
    emit reloadLogFont(fontConfigLog);
}

void SettingModule::settingSave() {
    const QJsonObject fontConfigLog = m_fontSettingLogWidget->settingExport();
    emit reloadLogFont(fontConfigLog);
    emit saveLogFont(fontConfigLog);
    accept();
}
