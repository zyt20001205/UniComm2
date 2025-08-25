#ifndef MANUAL_H
#define MANUAL_H

#include <QDialog>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextBrowser>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class Manual final : public QDialog {
    Q_OBJECT

public:
    explicit Manual(QWidget *parent = nullptr);

    ~Manual() override = default;

private:
    QTextBrowser *m_manualTextBrowser = nullptr;
};

#endif //MANUAL_H
