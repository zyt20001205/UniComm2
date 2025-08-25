#include "../include/manual.h"

Manual::Manual(QWidget *parent)
    : QDialog(parent) {
    this->setWindowTitle("Manual");
    this->resize(900, 600);
    const auto manualLayout = new QVBoxLayout(this); // NOLINT
    const auto manualSplitter = new QSplitter(Qt::Horizontal); // NOLINT
    manualLayout->addWidget(manualSplitter);

    const auto manualTreeView = new QTreeView(); // NOLINT
    manualSplitter->addWidget(manualTreeView);
    manualTreeView->setHeaderHidden(true);
    manualTreeView->setFont(QFont("Consolas", 12));

    const auto manualStandardItemModel = new QStandardItemModel(); // NOLINT
    manualTreeView->setModel(manualStandardItemModel);

    const auto manualPortStandardItem = new QStandardItem("port"); // NOLINT
    manualStandardItemModel->appendRow(manualPortStandardItem);
    const auto manualOpenStandardItem = new QStandardItem("open"); // NOLINT
    manualPortStandardItem->appendRow(manualOpenStandardItem);
    const auto manualCloseStandardItem = new QStandardItem("close"); // NOLINT
    manualPortStandardItem->appendRow(manualCloseStandardItem);
    const auto manualInfoStandardItem = new QStandardItem("info"); // NOLINT
    manualPortStandardItem->appendRow(manualInfoStandardItem);
    const auto manualWriteStandardItem = new QStandardItem("write"); // NOLINT
    manualPortStandardItem->appendRow(manualWriteStandardItem);
    const auto manualReadStandardItem = new QStandardItem("read"); // NOLINT
    manualPortStandardItem->appendRow(manualReadStandardItem);

    manualTreeView->expandAll();
    connect(manualTreeView, &QTreeView::clicked, [this](const QModelIndex &index) {
        if (const QString itemText = index.data(Qt::DisplayRole).toString(); itemText == "open") {
            m_manualTextBrowser->setSource(QUrl("open.md"));
        } else if (itemText == "close") {
            m_manualTextBrowser->setSource(QUrl("close.md"));
        } else if (itemText == "info") {
            m_manualTextBrowser->setSource(QUrl("info.md"));
        } else if (itemText == "write") {
            m_manualTextBrowser->setSource(QUrl("write.md"));
        } else if (itemText == "read") {
            m_manualTextBrowser->setSource(QUrl("read.md"));
        }
    });

    m_manualTextBrowser = new QTextBrowser();
    manualSplitter->addWidget(m_manualTextBrowser);
    m_manualTextBrowser->setOpenExternalLinks(true);
    m_manualTextBrowser->setSearchPaths(QStringList() << ":/doc");

    m_manualTextBrowser->document()->setDefaultFont(QFont("Consolas", 11));

    manualSplitter->setStretchFactor(0, 1);
    manualSplitter->setStretchFactor(1, 3);
}
