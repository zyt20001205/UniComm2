#include "../include/manual.h"

Manual::Manual(QWidget *parent)
    : QDialog(parent) {
    this->setWindowTitle("Manual");
    this->resize(900, 600);
    auto *manualLayout = new QVBoxLayout(this); // NOLINT
    auto *manualSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    manualLayout->addWidget(manualSplitter);

    auto *manualTreeView = new QTreeView(); // NOLINT
    manualSplitter->addWidget(manualTreeView);
    manualTreeView->setHeaderHidden(true);
    manualTreeView->setFont(QFont("Consolas", 12));

    auto *standardItemModel = new QStandardItemModel(); // NOLINT
    manualTreeView->setModel(standardItemModel);

    auto *portStandardItem = new QStandardItem("port"); // NOLINT
    standardItemModel->appendRow(portStandardItem);
    auto *standardItem = new QStandardItem("open"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.open", Qt::UserRole + 1);
    standardItem = new QStandardItem("close"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.close", Qt::UserRole + 1);
    standardItem = new QStandardItem("info"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.info", Qt::UserRole + 1);
    standardItem = new QStandardItem("writeText"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.writeText", Qt::UserRole + 1);
    standardItem = new QStandardItem("writeData"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.writeData", Qt::UserRole + 1);
    standardItem = new QStandardItem("readText"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.readText", Qt::UserRole + 1);
    standardItem = new QStandardItem("readData"); // NOLINT
    portStandardItem->appendRow(standardItem);
    standardItem->setData("port.readData", Qt::UserRole + 1);

    manualTreeView->expandAll();
    connect(manualTreeView, &QTreeView::clicked, [this](const QModelIndex &index) {
        const QString func = index.data(Qt::UserRole + 1).toString();
        manualShow(func);
    });

    m_manualTextBrowser = new QTextBrowser();
    manualSplitter->addWidget(m_manualTextBrowser);
    m_manualTextBrowser->setOpenExternalLinks(true);
    m_manualTextBrowser->setSearchPaths(QStringList() << ":/doc");

    m_manualTextBrowser->document()->setDefaultFont(QFont("Consolas", 11));

    manualSplitter->setStretchFactor(0, 1);
    manualSplitter->setStretchFactor(1, 3);
}

void Manual::manualShow(const QString &func) {
    static const QHash<QString, QUrl> manualMap = {
        {"port.open", QUrl("open.md")},
        {"port.close", QUrl("close.md")},
        {"port.info", QUrl("info.md")},
        {"port.writeText", QUrl("writeText.md")},
        {"port.writeData", QUrl("writeData.md")},
        {"port.readText", QUrl("readText.md")},
        {"port.readData", QUrl("readData.md")}
    };
    if (!manualMap.contains(func)) return;
    this->show();
    m_manualTextBrowser->setSource(manualMap.value(func));
}
